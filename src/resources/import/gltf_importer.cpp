#include "star/resources/import/gltf_importer.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include <cgltf.h>
#include <stb_image.h>

#include "star/core/logger.hpp"
#include "star/graphics/vertex.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/mesh/mesh.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/texture/texture.hpp"

namespace star::resources {
    namespace {
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

        void compute_normals(Mesh& mesh) {
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

        void compute_tangents(Mesh& mesh) {
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

        ResourceHandle<Texture> load_image_texture(ResourceManager& resources, const cgltf_data* data,
                                                   const cgltf_image* image, const std::filesystem::path& dir,
                                                   const std::string& name) {
            if (!image)
                return {};
            if (const auto existing = resources.texture_by_name(name); existing.is_valid())
                return existing;

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
                STAR_LOG_WARN(LogCategory::Resources, "glTF image '{}' could not be decoded", name);
                return {};
            }

            auto tex = std::make_unique<Texture>();
            tex->desc.width = static_cast<u32>(w);
            tex->desc.height = static_cast<u32>(h);
            tex->desc.format = TextureFormat::RGBA8;
            tex->desc.generate_mipmaps = false;
            tex->data.assign(pixels, pixels + static_cast<size_t>(w) * h * 4);
            stbi_image_free(pixels);
            return resources.create_texture(name, std::move(tex));
        }

        ResourceHandle<Material> import_material(ResourceManager& resources, const cgltf_data* data,
                                                 const cgltf_material* gm, const std::string& base,
                                                 const std::filesystem::path& dir) {
            if (!gm)
                return resources.default_material();
            const auto idx = static_cast<cgltf_size>(gm - data->materials);

            auto m = std::make_unique<Material>();
            m->shader = resources.default_shader();

            const auto load_view = [&](const cgltf_texture_view& view, const char* slot) -> ResourceHandle<Texture> {
                if (!view.texture || !view.texture->image)
                    return {};
                const auto img = static_cast<cgltf_size>(view.texture->image - data->images);
                return load_image_texture(resources, data, view.texture->image, dir,
                                          base + "#img" + std::to_string(img) + "_" + slot);
            };

            if (gm->has_pbr_metallic_roughness) {
                const auto& p = gm->pbr_metallic_roughness;
                m->albedo_color = {p.base_color_factor[0], p.base_color_factor[1], p.base_color_factor[2],
                                   p.base_color_factor[3]};
                m->metallic = p.metallic_factor;
                m->roughness = p.roughness_factor;
                m->albedo_texture = load_view(p.base_color_texture, "albedo");
                m->metallic_roughness_texture = load_view(p.metallic_roughness_texture, "mr");
            }
            m->normal_texture = load_view(gm->normal_texture, "normal");
            m->emissive_color = {gm->emissive_factor[0], gm->emissive_factor[1], gm->emissive_factor[2], 1.0f};
            m->emissive_texture = load_view(gm->emissive_texture, "emissive");

            const std::string name =
                base + "#mat" + std::to_string(idx) + (gm->name ? std::string{"_"} + gm->name : std::string{});
            return resources.create_material(name, std::move(m));
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
    } // namespace

    ImportedModel import_gltf(ResourceManager& resources, const std::filesystem::path& path) {
        const std::string path_str = path.string();
        cgltf_options options{};
        cgltf_data* data = nullptr;

        if (cgltf_parse_file(&options, path_str.c_str(), &data) != cgltf_result_success) {
            STAR_LOG_ERROR(LogCategory::Resources, "glTF parse failed: {}", path_str);
            return {};
        }
        if (cgltf_load_buffers(&options, data, path_str.c_str()) != cgltf_result_success) {
            STAR_LOG_ERROR(LogCategory::Resources, "glTF buffer load failed: {}", path_str);
            cgltf_free(data);
            return {};
        }
        if (cgltf_validate(data) != cgltf_result_success) {
            STAR_LOG_WARN(LogCategory::Resources, "glTF validation warnings: {}", path_str);
        }

        const std::string base = path.filename().string();
        const std::filesystem::path dir = path.parent_path();
        ImportedModel model;
        model.name = base;

        std::vector<ResourceHandle<Material>> mat_handles(data->materials_count);
        for (cgltf_size m = 0; m < data->materials_count; ++m)
            mat_handles[m] = import_material(resources, data, &data->materials[m], base, dir);

        std::vector<std::vector<ImportedPrimitive>> mesh_prims(data->meshes_count);
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

                Mesh mesh;
                mesh.vertices.resize(pos->count);
                for (cgltf_size i = 0; i < pos->count; ++i) {
                    Vertex v{};
                    v.position = read_vec3(pos, i);
                    v.normal = read_vec3(nrm, i);
                    v.tex_coords = read_vec2(uv, i);
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
                    compute_normals(mesh);
                if (!tan)
                    compute_tangents(mesh);
                for (cgltf_size k = 0; k + 2 < mesh.indices.size(); k += 3)
                    std::swap(mesh.indices[k + 1], mesh.indices[k + 2]);

                const std::string mesh_name = base + "#m" + std::to_string(mi) + "_p" + std::to_string(pi);
                const auto handle = resources.create_mesh(mesh_name, std::make_unique<Mesh>(std::move(mesh)));
                const auto mat = prim.material ? mat_handles[static_cast<cgltf_size>(prim.material - data->materials)]
                                               : resources.default_material();
                mesh_prims[mi].push_back({handle, mat});
            }
        }

        model.nodes.resize(data->nodes_count);
        for (cgltf_size ni = 0; ni < data->nodes_count; ++ni) {
            const cgltf_node& gn = data->nodes[ni];
            auto& [name, position, rotation, scale, primitives, children] = model.nodes[ni];
            name = gn.name ? gn.name : ("node_" + std::to_string(ni));

            if (gn.has_matrix) {
                decompose(gn.matrix, position, rotation, scale);
            } else {
                if (gn.has_translation)
                    position = {gn.translation[0], gn.translation[1], gn.translation[2]};
                if (gn.has_rotation)
                    rotation = {gn.rotation[0], gn.rotation[1], gn.rotation[2], gn.rotation[3]};
                if (gn.has_scale)
                    scale = {gn.scale[0], gn.scale[1], gn.scale[2]};
            }

            if (gn.mesh) {
                const auto midx = static_cast<cgltf_size>(gn.mesh - data->meshes);
                primitives = mesh_prims[midx];
            }

            children.reserve(gn.children_count);
            for (cgltf_size c = 0; c < gn.children_count; ++c)
                children.push_back(static_cast<u32>(gn.children[c] - data->nodes));
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

        const cgltf_size prim_total = [&] {
            cgltf_size n = 0;
            for (const auto& mp : mesh_prims)
                n += mp.size();
            return n;
        }();
        STAR_LOG_INFO(LogCategory::Resources, "Imported glTF '{}': {} nodes, {} meshes, {} primitives, {} materials",
                      base, data->nodes_count, data->meshes_count, prim_total, data->materials_count);

        cgltf_free(data);
        return model;
    }
} // namespace star::resources
