#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace star::application {
    class STAR_EXPORT CommandLineArgs {
      public:
        CommandLineArgs() = default;
        CommandLineArgs(int argc, const char* argv[]);

        void parse(int argc, const char* argv[]);

        bool has_flag(const std::string& flag) const;

        std::string get_value(const std::string& key, const std::string& default_value = "") const;

        i32 get_int(const std::string& key, i32 default_value = 0) const;

        f32 get_float(const std::string& key, f32 default_value = 0.0f) const;

        bool get_bool(const std::string& key, bool default_value = false) const;

        const std::vector<std::string>& get_positional() const {
            return m_positional_args;
        }

        const std::vector<std::string>& get_all() const {
            return m_all_args;
        }

        const std::string& get_executable_path() const {
            return m_executable_path;
        }

        bool empty() const {
            return m_all_args.empty();
        }

        size_t size() const {
            return m_all_args.size();
        }

      private:
        std::string m_executable_path;
        std::vector<std::string> m_all_args;
        std::vector<std::string> m_positional_args;
        std::unordered_map<std::string, std::string> m_key_value_args;
        std::unordered_map<std::string, bool> m_flags;
    };
} // namespace star::application
