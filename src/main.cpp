#include <cpptrace/cpptrace.hpp>
#include <csignal>
#include <siplus/siplus.h>

#include <iostream>
#include <fstream>

struct stream_wrapper {
    using deleter = std::function<void ()>;

    stream_wrapper(std::ostream& out) : ostream_(out) {}
    stream_wrapper(std::ostream& out, deleter deleter) : ostream_(out), deleter_(deleter) {}

    template<typename T>
    stream_wrapper& operator<<(T&& x) {
        ostream_ << x;
        return *this;
    }

    stream_wrapper& operator<<(std::ostream& (*func)(std::ostream&)) {
        ostream_ << func;
        return *this;
    }

    ~stream_wrapper() {
        if(deleter_) {
            deleter_();
        }
    }

private:
    std::ostream& ostream_;
    deleter deleter_;
};

struct Args {
    std::string input;
    std::unique_ptr<stream_wrapper> out;
    SIPlus::text::UnknownDataTypeContainer default_value;
    std::unordered_map<std::string, SIPlus::text::UnknownDataTypeContainer> args;
};

void help() {
    std::cout 
        << "siplus: SIPlus String Interpolation CLI Utility\n"
        << "usage: siplus <template> -d <default> [-v VAL=<value>]\n"
        << "\n"
        << "Arguments:\n"
        << "\t-h\t--help\t\t\tShow this help message.\n"
        << "\t-i\t--input <file>\t\tUse input file instead of raw template text.\n"
        << "\t-v\t--val <name>=<value>\tSet value for use in template. This will be made available as a global.\n"
        << "\t-d\t--default <value>\tSet value for use in template. This will be made available as a the default value.\n"
        << "\t-o\t--output <file>\t\tOutput to file instead of to stdout.\n"
        << "\n"
        << "Examples:"
        << "    siplus \"Hello, {$first} {$last}\" -v first=john -v last=doe\n"
        << "    # Hello, john doe\n"
        << "\n"
        << "    siplus \"Hello, {.}\" -d world\n"
        << "    # Hello, world\n"
        << "\n"
        << "    siplus -i hello.txt -v first=world\n"
        << "    # Hello, world\n"
        << std::endl;
}

void assert(bool test, const std::string& msg) {
    if(!test) {
        throw std::runtime_error{msg};
    }
}

std::string read_file(const std::string& file) {
    std::ifstream input{file};
    std::stringstream ss;
    ss << input.rdbuf();
    return ss.str();
}

std::pair<std::string, std::string> parse_val(const std::string& val) {
    auto idx = val.find('=');
    assert(idx != std::string::npos, "Could not find value in " + val);

    auto name = val.substr(0, idx);
    auto value = val.substr(idx + 1);

    return {name, value};
}

void parse_args(Args& args, int argc, char **argv) {
    bool input_specified = false;
    bool output_specified = false;
    bool default_specified = false;

    for(int i = 1; i < argc; i++) {
        std::string val = argv[i];

        if(val == "-h" || val == "--help") {
            help();
            std::exit(0);

        } else if(val == "-i" || val == "--input") {
            assert(!input_specified, "input was already specified, -i was unexpected");
            assert(i + 1 < argc, "expected argument for -i");
            input_specified = true;
            args.input = read_file(argv[++i]);

        } else if(val == "-v" || val == "--val") {
            assert(i + 1 < argc, "expected argument for -v");

            auto [name, value] = parse_val(argv[++i]);
            assert(args.args.find(name) == args.args.end(), "value " + name + " was already specified");

            args.args[name] = SIPlus::text::make_data(value);

        } else if(val == "-d" || val == "--default") {
            assert(!default_specified, "default value already specified");
            assert(i + 1 < argc, "expected argument for -d");

            default_specified = true;
            args.default_value = SIPlus::text::make_data(std::string{argv[++i]});

        } else if(val == "-o" || val == "--output") {
            assert(!output_specified, "output already specified");
            assert(i + 1 < argc, "expected argument for -o");

            output_specified = true;
            auto ptr = new std::ofstream{argv[++i]};
            args.out = std::make_unique<stream_wrapper>(*ptr, [ptr]() {
                delete ptr;
            });

        } else if(!input_specified) {
            input_specified = true;
            args.input = val;

        } else {
            throw std::runtime_error{"Unknown extra parameter " + val};
        }
    }

    if(!output_specified) {
        args.out = std::make_unique<stream_wrapper>(std::cout);
    }

    assert(input_specified, "No input specified (-h for help)");
    assert(default_specified, "No default value specified");
}

void signal_handler(int signal) {
    std::cout << "Signal " << signal << " caught. Death imminent.";
    cpptrace::generate_trace().print();
    std::exit(1);
}

int main(int argc, char **argv) {
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGABRT, signal_handler);

    Args args;

    try {
        parse_args(args, argc, argv);
    } catch(std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 2;
    }

    SIPlus::Parser parser;
    SIPlus::ParseOpts opts;

    for(auto [k, _] : args.args) {
        opts.globals.push_back(k);
    }

    SIPlus::text::TextConstructor constructor;
    try {
        constructor = parser.get_interpolation(args.input, opts);
    } catch(std::exception& e) {
        std::cerr << "parse error: " << e.what() << std::endl;
        return 1;
    }

    auto ctxBuilder = parser.context().builder().use_default(args.default_value);
    for(auto [k, v] : args.args) {
        ctxBuilder.with(k, v);
    }

    std::string text;
    try {
        text = constructor.construct_with(ctxBuilder.build());
    } catch(std::exception& e) {
        std::cerr << "execution error: " << e.what() << std::endl;
        return 1;
    }

    *args.out << text << std::flush;

    return 0;
}
