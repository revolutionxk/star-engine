#pragma once
#include <string>
#include <vector>

namespace star::graphics {
    struct ShaderDescriptor {
        enum class Stage {
            Vertex,
            Fragment,
            Compute
        };

        struct ShaderStage {
            Stage stage;
            std::vector<u8> bytecode;
            std::string entry_point = "main";
        };

        std::vector<ShaderStage> stages;
        std::string name;
    };

    struct Shader {
        virtual ~Shader() = default;
    };
} // namespace star::graphics
