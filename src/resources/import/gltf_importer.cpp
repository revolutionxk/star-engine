#include "star/resources/import/gltf_importer.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

#include <cgltf.h>
#include <stb_image.h>

#include "star/core/logger.hpp"
#include "star/graphics/vertex.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/mesh/mesh.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/texture/texture.hpp"

namespace star::resources {
    namespace detail {
        Vector3 read_vec3(const cgltf_accessor* acc, const cgltf_size i) {
            float v[3] = {0.0f, 0.0f, 0.0f};
            if (acc)
                cgltf_accessor_read_float(acc, i, v, 3);
            return {v[0], v[1], v[2]};
        }

        Vector2 read_vec2(const cgltf_accessor* acc, const cgltf_size i) {
            float v[2] = {0.0f, 0.0f};
            if (acc)
                cgltf_accessor_read_float(acc, i, v, 2);
            return {v[0], v[1]};
        }

        void compute_normals(RawMesh& mesh) {
            for (auto& v : mesh.vertices)
                v.normal = Vector3{0.0f, 0.0f, 0.0f};
            for (cgltf_size t = 0; t + 2 < mesh.indices.size(); t += 3) {
                const u32 i0 = mesh.indices[t], i1 = mesh.indices[t + 1], i2 = mesh.indices[t + 2];
                const Vector3 e1 = mesh.vertices[i1].position - mesh.vertices[i0].position;
                const Vector3 e2 = mesh.vertices[i2].position - mesh.vertices[i0].position;
                const Vector3 n = e1.cross(e2);
                mesh.vertices[i0].normal += n;
                mesh.vertices[i1].normal += n;
                mesh.vertices[i2].normal += n;
            }
            for (auto& v : mesh.vertices) {
                if (v.normal.length_squared() > 1e-12f)
                    v.normal = v.normal.normalized();
                else
                    v.normal = Vector3{0.0f, 1.0f, 0.0f};
            }
        }

        void compute_tangents(RawMesh& mesh) {
            std::vector<Vector3> tan(mesh.vertices.size(), Vector3{0.0f, 0.0f, 0.0f});
            std::vector<Vector3> bit(mesh.vertices.size(), Vector3{0.0f, 0.0f, 0.0f});
            for (cgltf_size t = 0; t + 2 < mesh.indices.size(); t += 3) {
                const u32 i0 = mesh.indices[t], i1 = mesh.indices[t + 1], i2 = mesh.indices[t + 2];
                const Vector3 p0 = mesh.vertices[i0].position;
                const Vector3 e1 = mesh.vertices[i1].position - p0;
                const Vector3 e2 = mesh.vertices[i2].position - p0;
                const Vector2 uv0 = mesh.vertices[i0].tex_coords;
                const Vector2 duv1{mesh.vertices[i1].tex_coords.x - uv0.x, mesh.vertices[i1].tex_coords.y - uv0.y};
                const Vector2 duv2{mesh.vertices[i2].tex_coords.x - uv0.x, mesh.vertices[i2].tex_coords.y - uv0.y};
                const float det = duv1.x * duv2.y - duv2.x * duv1.y;
                if (std::abs(det) < 1e-8f)
                    continue;
                const float r = 1.0f / det;
                const Vector3 tdir = (e1 * duv2.y - e2 * duv1.y) * r;
                const Vector3 bdir = (e2 * duv1.x - e1 * duv2.x) * r;
                tan[i0] += tdir;
                tan[i1] += tdir;
                tan[i2] += tdir;
                bit[i0] += bdir;
                bit[i1] += bdir;
                bit[i2] += bdir;
            }
            for (cgltf_size i = 0; i < mesh.vertices.size(); ++i) {
                const Vector3 n = mesh.vertices[i].normal;
                Vector3 t = tan[i] - n * n.dot(tan[i]);
                if (t.length_squared() < 1e-12f) {
                    const Vector3 axis = std::abs(n.x) < 0.9f ? Vector3{1.0f, 0.0f, 0.0f} : Vector3{0.0f, 1.0f, 0.0f};
                    t = axis - n * n.dot(axis);
                }
                t = t.normalized();
                const float w = n.cross(t).dot(bit[i]) < 0.0f ? -1.0f : 1.0f;
                mesh.vertices[i].tangent = t;
                mesh.vertices[i].bitangent = n.cross(t) * w;
            }
        }

        bool decode_image(const cgltf_image* image, const std::filesystem::path& dir, RawTexture& out) {
            if (!image)
                return false;

            int w = 0, h = 0, comp = 0;
            stbi_uc* pixels = nullptr;

            if (image->buffer_view) {
                const cgltf_buffer_view* bv = image->buffer_view;
                const auto* bytes = static_cast<const stbi_uc*>(bv->buffer->data) + bv->offset;
                pixels = stbi_load_from_memory(bytes, static_cast<int>(bv->size), &w, &h, &comp, 4);
            } else if (image->uri) {
                if (std::strncmp(image->uri, "data:", 5) == 0) {
                    if (const char* comma = std::strchr(image->uri, ',')) {
                        const char* b64 = comma + 1;
                        const cgltf_size len = std::strlen(b64);
                        cgltf_size size = len / 4 * 3;
                        if (len >= 2 && b64[len - 1] == '=')
                            --size;
                        if (len >= 2 && b64[len - 2] == '=')
                            --size;
                        constexpr cgltf_options opt{};
                        void* decoded = nullptr;
                        if (cgltf_load_buffer_base64(&opt, size, b64, &decoded) == cgltf_result_success && decoded) {
                            pixels = stbi_load_from_memory(static_cast<stbi_uc*>(decoded), static_cast<int>(size), &w,
                                                           &h, &comp, 4);
                            std::free(decoded);
                        }
                    }
                } else {
                    std::string uri = image->uri;
                    uri.push_back('\0');
                    cgltf_decode_uri(uri.data());
                    const std::filesystem::path file = dir / uri.c_str();
                    pixels = stbi_load(file.string().c_str(), &w, &h, &comp, 4);
                }
            }

            if (!pixels) {
                return false;
            }

            out.width = static_cast<u32>(w);
            out.height = static_cast<u32>(h);
            out.pixels.assign(pixels, pixels + static_cast<size_t>(w) * h * 4);
            stbi_image_free(pixels);
            return true;
        }

        i32 texture_index(const cgltf_texture_view& view, const cgltf_data* data,
                          const std::filesystem::path& dir, const std::string& base, const char* slot,
                          std::vector<RawTexture>& textures, std::unordered_map<cgltf_size, i32>& cache) {
            if (!view.texture || !view.texture->image)
                return -1;

            const auto img = static_cast<cgltf_size>(view.texture->image - data->images);
            if (const auto it = cache.find(img); it != cache.end())
                return it->second;

            RawTexture raw;
            raw.name = base + "#img" + std::to_string(img) + "_" + slot;
            if (!decode_image(view.texture->image, dir, raw)) {
                STAR_LOG_WARN(LogCategory::Resources, "glTF image '{}' could not be decoded", raw.name);
                cache.emplace(img, -1);
                return -1;
            }

            const auto index = static_cast<i32>(textures.size());
            textures.push_back(std::move(raw));
            cache.emplace(img, index);
            return index;
        }

        RawMaterial parse_material(const cgltf_data* data, const cgltf_material* gm, const std::string& base,
                                   const std::filesystem::path& dir, std::vector<RawTexture>& textures,
                                   std::unordered_map<cgltf_size, i32>& cache) {
            RawMaterial m;
            const auto idx = static_cast<cgltf_size>(gm - data->materials);
            m.name = base + "#mat" + std::to_string(idx) + (gm->name ? std::string{"_"} + gm->name : std::string{});

            if (gm->has_pbr_metallic_roughness) {
                const auto& p = gm->pbr_metallic_roughness;
                m.albedo_color = {p.base_color_factor[0], p.base_color_factor[1], p.base_color_factor[2],
                                  p.base_color_factor[3]};
                m.metallic = p.metallic_factor;
                m.roughness = p.roughness_factor;
                m.albedo_texture = texture_index(p.base_color_texture, data, dir, base, "albedo", textures, cache);
                m.metallic_roughness_texture =
                    texture_index(p.metallic_roughness_texture, data, dir, base, "mr", textures, cache);
            }
            m.normal_texture = texture_index(gm->normal_texture, data, dir, base, "normal", textures, cache);
            m.emissive_color = {gm->emissive_factor[0], gm->emissive_factor[1], gm->emissive_factor[2], 1.0f};
            m.emissive_texture = texture_index(gm->emissive_texture, data, dir, base, "emissive", textures, cache);
            return m;
        }

        void decompose(const cgltf_float m[16], Vector3& t, Quaternion& r, Vector3& s) {
            t = {m[12], m[13], m[14]};
            Vector3 c0{m[0], m[1], m[2]};
            Vector3 c1{m[4], m[5], m[6]};
            Vector3 c2{m[8], m[9], m[10]};
            s = {c0.length(), c1.length(), c2.length()};
            if (s.x > 1e-8f)
                c0 = c0 / s.x;
            if (s.y > 1e-8f)
                c1 = c1 / s.y;
            if (s.z > 1e-8f)
                c2 = c2 / s.z;

            const float trace = c0.x + c1.y + c2.z;
            Quaternion q{0.0f, 0.0f, 0.0f, 1.0f};
            if (trace > 0.0f) {
                float k = std::sqrt(trace + 1.0f) * 2.0f;
                q.w = 0.25f * k;
                q.x = (c1.z - c2.y) / k;
                q.y = (c2.x - c0.z) / k;
                q.z = (c0.y - c1.x) / k;
            } else if (c0.x > c1.y && c0.x > c2.z) {
                float k = std::sqrt(1.0f + c0.x - c1.y - c2.z) * 2.0f;
                q.w = (c1.z - c2.y) / k;
                q.x = 0.25f * k;
                q.y = (c1.x + c0.y) / k;
                q.z = (c2.x + c0.z) / k;
            } else if (c1.y > c2.z) {
                float k = std::sqrt(1.0f + c1.y - c0.x - c2.z) * 2.0f;
                q.w = (c2.x - c0.z) / k;
                q.x = (c1.x + c0.y) / k;
                q.y = 0.25f * k;
                q.z = (c2.y + c1.z) / k;
            } else {
                float k = std::sqrt(1.0f + c2.z - c0.x - c1.y) * 2.0f;
                q.w = (c0.y - c1.x) / k;
                q.x = (c2.x + c0.z) / k;
                q.y = (c2.y + c1.z) / k;
                q.z = 0.25f * k;
            }
            r = q;
        }
    } // namespace detail

    RawModel parse_gltf(const std::filesystem::path& path) {
        const std::string path_str = path.string();
        cgltf_options options{};
        cgltf_data* data = nullptr;

        RawModel model;

        if (cgltf_parse_file(&options, path_str.c_str(), &data) != cgltf_result_success) {
            STAR_LOG_ERROR(LogCategory::Resources, "glTF parse failed: {}", path_str);
            return model;
        }
        if (cgltf_load_buffers(&options, data, path_str.c_str()) != cgltf_result_success) {
            STAR_LOG_ERROR(LogCategory::Resources, "glTF buffer load failed: {}", path_str);
            cgltf_free(data);
            return model;
        }
        if (cgltf_validate(data) != cgltf_result_success) {
            STAR_LOG_WARN(LogCategory::Resources, "glTF validation warnings: {}", path_str);
        }

        const std::string base = path.filename().string();
        const std::filesystem::path dir = path.parent_path();
        model.name = base;

        std::unordered_map<cgltf_size, i32> texture_cache;
        model.materials.reserve(data->materials_count);
        for (cgltf_size m = 0; m < data->materials_count; ++m) {
            model.materials.push_back(
                detail::parse_material(data, &data->materials[m], base, dir, model.textures, texture_cache));
        }

        std::vector<std::vector<RawPrimitive>> mesh_prims(data->meshes_count);
        for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
            const cgltf_mesh& gmesh = data->meshes[mi];
            for (cgltf_size pi = 0; pi < gmesh.primitives_count; ++pi) {
                const cgltf_primitive& prim = gmesh.primitives[pi];
                if (prim.type != cgltf_primitive_type_triangles)
                    continue;

                const cgltf_accessor *pos = nullptr, *nrm = nullptr, *uv = nullptr, *tan = nullptr;
                for (cgltf_size a = 0; a < prim.attributes_count; ++a) {
                    const cgltf_attribute& attr = prim.attributes[a];
                    switch (attr.type) {
                        case cgltf_attribute_type_position:
                            pos = attr.data;
                            break;
                        case cgltf_attribute_type_normal:
                            nrm = attr.data;
                            break;
                        case cgltf_attribute_type_texcoord:
                            if (attr.index == 0)
                                uv = attr.data;
                            break;
                        case cgltf_attribute_type_tangent:
                            tan = attr.data;
                            break;
                        default:
                            break;
                    }
                }
                if (!pos || pos->count == 0)
                    continue;

                RawMesh mesh;
                mesh.name = base + "#m" + std::to_string(mi) + "_p" + std::to_string(pi);
                mesh.vertices.resize(pos->count);
                for (cgltf_size i = 0; i < pos->count; ++i) {
                    graphics::Vertex v{};
                    v.position = detail::read_vec3(pos, i);
                    v.normal = detail::read_vec3(nrm, i);
                    v.tex_coords = detail::read_vec2(uv, i);
                    if (tan) {
                        float t[4] = {0.0f, 0.0f, 0.0f, 1.0f};
                        cgltf_accessor_read_float(tan, i, t, 4);
                        v.tangent = {t[0], t[1], t[2]};
                        v.bitangent = v.normal.cross(v.tangent) * t[3];
                    }
                    mesh.vertices[i] = v;
                }

                if (prim.indices && prim.indices->count > 0) {
                    mesh.indices.resize(prim.indices->count);
                    for (cgltf_size k = 0; k < prim.indices->count; ++k)
                        mesh.indices[k] = static_cast<u32>(cgltf_accessor_read_index(prim.indices, k));
                } else {
                    mesh.indices.resize(pos->count);
                    for (cgltf_size k = 0; k < pos->count; ++k)
                        mesh.indices[k] = static_cast<u32>(k);
                }

                if (!nrm)
                    detail::compute_normals(mesh);
                if (!tan)
                    detail::compute_tangents(mesh);
                for (cgltf_size k = 0; k + 2 < mesh.indices.size(); k += 3)
                    std::swap(mesh.indices[k + 1], mesh.indices[k + 2]);

                const auto mesh_index = static_cast<i32>(model.meshes.size());
                model.meshes.push_back(std::move(mesh));

                const i32 material_index =
                    prim.material ? static_cast<i32>(prim.material - data->materials) : -1;
                mesh_prims[mi].push_back({mesh_index, material_index});
            }
        }

        model.nodes.resize(data->nodes_count);
        for (cgltf_size ni = 0; ni < data->nodes_count; ++ni) {
            const cgltf_node& gn = data->nodes[ni];
            RawNode& node = model.nodes[ni];
            node.name = gn.name ? gn.name : ("node_" + std::to_string(ni));

            if (gn.has_matrix) {
                detail::decompose(gn.matrix, node.position, node.rotation, node.scale);
            } else {
                if (gn.has_translation)
                    node.position = {gn.translation[0], gn.translation[1], gn.translation[2]};
                if (gn.has_rotation)
                    node.rotation = {gn.rotation[0], gn.rotation[1], gn.rotation[2], gn.rotation[3]};
                if (gn.has_scale)
                    node.scale = {gn.scale[0], gn.scale[1], gn.scale[2]};
            }

            if (gn.mesh) {
                const auto midx = static_cast<cgltf_size>(gn.mesh - data->meshes);
                node.primitives = mesh_prims[midx];
            }

            node.children.reserve(gn.children_count);
            for (cgltf_size c = 0; c < gn.children_count; ++c)
                node.children.push_back(static_cast<u32>(gn.children[c] - data->nodes));
        }

        const cgltf_scene* scene = data->scene ? data->scene : (data->scenes_count ? &data->scenes[0] : nullptr);
        if (scene) {
            for (cgltf_size k = 0; k < scene->nodes_count; ++k)
                model.roots.push_back(static_cast<u32>(scene->nodes[k] - data->nodes));
        } else {
            for (cgltf_size ni = 0; ni < data->nodes_count; ++ni)
                if (!data->nodes[ni].parent)
                    model.roots.push_back(static_cast<u32>(ni));
        }

        STAR_LOG_INFO(LogCategory::Resources,
                      "Parsed glTF '{}': {} nodes, {} meshes, {} materials, {} textures", base, model.nodes.size(),
                      model.meshes.size(), model.materials.size(), model.textures.size());

        cgltf_free(data);
        model.ok = true;
        return model;
    }

    ImportedModel upload_gltf(ResourceManager& resources, const RawModel& raw) {
        ImportedModel model;
        if (!raw.ok)
            return model;

        model.name = raw.name;
        model.roots = raw.roots;

        std::vector<ResourceHandle<Texture>> textures(raw.textures.size());
        for (std::size_t i = 0; i < raw.textures.size(); ++i) {
            const RawTexture& raw_texture = raw.textures[i];
            if (const auto existing = resources.texture_by_name(raw_texture.name); existing.is_valid()) {
                textures[i] = existing;
                continue;
            }

            auto texture = std::make_unique<Texture>();
            texture->desc.width = raw_texture.width;
            texture->desc.height = raw_texture.height;
            texture->desc.format = TextureFormat::RGBA8;
            texture->desc.generate_mipmaps = false;
            texture->data = raw_texture.pixels;
            textures[i] = resources.create_texture(raw_texture.name, std::move(texture));
        }

        const auto resolve_texture = [&](const i32 index) -> ResourceHandle<Texture> {
            return index >= 0 && static_cast<std::size_t>(index) < textures.size() ? textures[index]
                                                                                  : ResourceHandle<Texture>{};
        };

        std::vector<ResourceHandle<Material>> materials(raw.materials.size());
        for (std::size_t i = 0; i < raw.materials.size(); ++i) {
            const RawMaterial& raw_material = raw.materials[i];
            if (const auto existing = resources.material_by_name(raw_material.name); existing.is_valid()) {
                materials[i] = existing;
                continue;
            }

            auto material = std::make_unique<Material>();
            material->shader = resources.default_shader();
            material->albedo_color = raw_material.albedo_color;
            material->emissive_color = raw_material.emissive_color;
            material->metallic = raw_material.metallic;
            material->roughness = raw_material.roughness;
            material->albedo_texture = resolve_texture(raw_material.albedo_texture);
            material->normal_texture = resolve_texture(raw_material.normal_texture);
            material->metallic_roughness_texture = resolve_texture(raw_material.metallic_roughness_texture);
            material->emissive_texture = resolve_texture(raw_material.emissive_texture);
            materials[i] = resources.create_material(raw_material.name, std::move(material));
        }

        std::vector<ResourceHandle<Mesh>> meshes(raw.meshes.size());
        for (std::size_t i = 0; i < raw.meshes.size(); ++i) {
            const RawMesh& raw_mesh = raw.meshes[i];
            if (const auto existing = resources.mesh_by_name(raw_mesh.name); existing.is_valid()) {
                meshes[i] = existing;
                continue;
            }

            auto mesh = std::make_unique<Mesh>();
            mesh->vertices = raw_mesh.vertices;
            mesh->indices = raw_mesh.indices;
            meshes[i] = resources.create_mesh(raw_mesh.name, std::move(mesh));
        }

        model.nodes.resize(raw.nodes.size());
        for (std::size_t i = 0; i < raw.nodes.size(); ++i) {
            const RawNode& raw_node = raw.nodes[i];
            ImportedNode& node = model.nodes[i];
            node.name = raw_node.name;
            node.position = raw_node.position;
            node.rotation = raw_node.rotation;
            node.scale = raw_node.scale;
            node.children = raw_node.children;

            node.primitives.reserve(raw_node.primitives.size());
            for (const auto& [mesh_index, material_index] : raw_node.primitives) {
                if (mesh_index < 0 || static_cast<std::size_t>(mesh_index) >= meshes.size())
                    continue;
                const auto material = material_index >= 0 && static_cast<std::size_t>(material_index) < materials.size()
                                          ? materials[material_index]
                                          : resources.default_material();
                node.primitives.push_back({meshes[mesh_index], material});
            }
        }

        return model;
    }

    ImportedModel import_gltf(ResourceManager& resources, const std::filesystem::path& path) {
        return upload_gltf(resources, parse_gltf(path));
    }
} // namespace star::resources
