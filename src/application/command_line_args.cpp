#include "star/application/command_line_args.hpp"

#include <algorithm>
#include <sstream>

namespace star::application {
    CommandLineArgs::CommandLineArgs(const int argc, const char* argv[]) {
        parse(argc, argv);
    }

    void CommandLineArgs::parse(const int argc, const char* argv[]) {
        m_all_args.clear();
        m_positional_args.clear();
        m_key_value_args.clear();
        m_flags.clear();

        if (argc <= 0 || !argv) {
            return;
        }

        m_executable_path = argv[0];
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            m_all_args.push_back(arg);

            if (arg.empty()) {
                continue;
            }

            if (arg[0] == '-') {
                std::string key = arg;
                size_t dash_count = 0;
                while (dash_count < key.length() && key[dash_count] == '-') {
                    ++dash_count;
                }
                key = key.substr(dash_count);

                if (const size_t equals_pos = key.find('='); equals_pos != std::string::npos) {
                    std::string value = key.substr(equals_pos + 1);
                    key = key.substr(0, equals_pos);
                    m_key_value_args[key] = value;
                    m_flags[key] = true;
                } else {
                    if (i + 1 < argc && argv[i + 1][0] != '-') {
                        m_key_value_args[key] = argv[i + 1];
                        m_flags[key] = true;
                        ++i; // Skip next argument as it's the value
                        m_all_args.push_back(argv[i]);
                    } else {
                        // Just a flag: --flag
                        m_flags[key] = true;
                    }
                }
            } else {
                m_positional_args.push_back(arg);
            }
        }
    }

    bool CommandLineArgs::has_flag(const std::string& flag) const {
        const auto it = m_flags.find(flag);
        return it != m_flags.end() && it->second;
    }

    std::string CommandLineArgs::get_value(const std::string& key, const std::string& default_value) const {
        const auto it = m_key_value_args.find(key);
        return it != m_key_value_args.end() ? it->second : default_value;
    }

    i32 CommandLineArgs::get_int(const std::string& key, i32 default_value) const {
        auto it = m_key_value_args.find(key);
        if (it == m_key_value_args.end()) {
            return default_value;
        }

        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_value;
        }
    }

    f32 CommandLineArgs::get_float(const std::string& key, f32 default_value) const {
        auto it = m_key_value_args.find(key);
        if (it == m_key_value_args.end()) {
            return default_value;
        }

        try {
            return std::stof(it->second);
        } catch (...) {
            return default_value;
        }
    }

    bool CommandLineArgs::get_bool(const std::string& key, const bool default_value) const {
        const auto it = m_key_value_args.find(key);
        if (it == m_key_value_args.end()) {
            return default_value;
        }

        // Convert string to lowercase for comparison
        std::string value = it->second;
        std::ranges::transform(value, value.begin(), tolower);

        if (value == "true" || value == "1" || value == "yes" || value == "on") {
            return true;
        }
        if (value == "false" || value == "0" || value == "no" || value == "off") {
            return false;
        }

        return default_value;
    }
} // namespace star::application
