#include <string.h>
#include <stdlib.h>
#include <map>
#include <fcntl.h>
#include <fmt/format.h>

#include "backend/c_code.h"

#include "logger.h"
#include "parser.c"
#include "registers.h"
#include "machine.h"

static bool valid = true;
static Backend* backend = nullptr;

static int line = 1;
static bool pByLine = true;

void FailError(const char* ts, const char* te);

%%{
    machine gifscript;

    # End cmd
    action semi_tok{
        Parse(lparser, 0, 0, &valid);
    }

    # Registers
    action rgbaq_tok {
        Parse(lparser, REG, new std::any(GifRegisters::RGBAQ), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action xyz2_tok {
        Parse(lparser, REG, new std::any(GifRegisters::XYZ2), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action prim_tok {
        Parse(lparser, REG, new std::any(GifRegisters::PRIM), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action fog_tok {
        Parse(lparser, REG, new std::any(GifRegisters::FOG), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action fogcol_tok {
        Parse(lparser, REG, new std::any(GifRegisters::FOGCOL), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action scissor_tok {
        Parse(lparser, REG, new std::any(GifRegisters::SCISSOR), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action signal_tok {
        Parse(lparser, REG, new std::any(GifRegisters::SIGNAL), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action finish_tok {
        Parse(lparser, REG, new std::any(GifRegisters::FINISH), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    # Modifiers
    # Primitive Types
    action mod_point_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Point), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_line_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Line), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_linestrip_tok {
        Parse(lparser, MOD, new std::any(RegModifier::LineStrip), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_triangle_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Triangle), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_trianglestrip_tok {
        Parse(lparser, MOD, new std::any(RegModifier::TriangleStrip), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_trianglefan_tok {
        Parse(lparser, MOD, new std::any(RegModifier::TriangleFan), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_sprite_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Sprite), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    # Primitive Modifiers
    action mod_gouraud_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Gouraud), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_fogging_tok {
        Parse(lparser, MOD, new std::any(RegModifier::Fogging), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action mod_aa1_tok {
        Parse(lparser, MOD, new std::any(RegModifier::AA1), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    # Vectors
    action vec4_tok {
        std::string s(ts, te - ts);
        Parse(lparser, VEC4, new std::any(Vec4(s)), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action vec3_tok {
        std::string s(ts, te - ts);
        Parse(lparser, VEC3, new std::any(Vec3(s)), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    action vec2_tok {
        std::string s(ts, te - ts);
        Parse(lparser, VEC2, new std::any(Vec2(s)), &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    # Integer Constants
    action int_const_tok {
        std::string s(ts, te - ts);
        Parse(lparser, NUMBER_LITERAL, new std::any(std::stoi(s)), &valid);
        if(!valid)
        {
            FailError(ts, te);
        }
    }

    # Hexadecimal Constants
    action hex_const_tok {
        std::string s(ts, te - ts);
        Parse(lparser, NUMBER_LITERAL, new std::any(std::stoi(s, nullptr, 16)), &valid);
        if(!valid)
        {
            FailError(ts, te);
        }
    }

    # Block controls
    action block_begin_tok {
        Parse(lparser, BLOCK_START, 0, &valid);
        if(!valid) {
            FailError(ts, te);
        }
        Parse(lparser, 0, 0, &valid);
    }

    action block_end_tok {
        if(!valid && !pByLine) {
            logger::warn("!!!! Ending block when in an invalid state. Your output may be incorrect. Please try without --oneshot-parse");
        }
        Parse(lparser, BLOCK_END, 0, &valid);
        if(!valid) {
            FailError(ts, te);
        }
        Parse(lparser, 0, 0, &valid);
    }

    # Macro keyword
    action macro_tok {
        Parse(lparser, MACRO, 0, &valid);
        if(!valid) {
            FailError(ts, te);
        }
    }

    # Identifiers
    action identifier_tok {
        Parse(lparser, IDENTIFIER, new std::any(std::string(ts, te)), &valid);
        if(!valid)
        {
            FailError(ts, te);
        }
    }

    # Floating literals.
    fract_const = digit* '.' digit+ | digit+ '.';
    exponent = [eE] [+\-]? digit+;
    float_suffix = [flFL];

    c_comment := 
        any* :>> '*/'
        @{ fgoto main; };

    # End cmd
    semi = ';';

    # Registers
    rgbaq = (/rgbaq/i|/rgba/i);
    xyz2 = /xyz2/i;
    prim = /prim/i;
    fog = /fog/i;
    fogcol = /fogcol/i;
    scissor = /scissor/i;
    signal = /signal/i;
    finish = /finish/i;

    # Modifiers
        # Primitive Types
        mod_point = /point/i;
        mod_line = /line/i;
        mod_linestrip = /linestrip/i;
        mod_triangle = (/triangle/i|/tri/i);
        mod_trianglestrip = (/trianglestrip/i|/tristrip/i);
        mod_trianglefan = (/trianglefan/i|/trifan/i);
        mod_sprite = (/sprite/i|/rect/i|/quad/i);
        # Primitive Modifiers
        mod_gouraud = /gouraud/i;
        mod_fogging = (/fogging/i|/fog/i);
        mod_aa1 = /aa1/i;

    # Vectors
    vec4 = [0-9a-fA-F]+ ('.' [0-9]+)? ',' [0-9a-fA-F]+ ('.' [0-9]+)? ',' [0-9a-fA-F]+ ('.' [0-9]+)? ',' [0-9a-fA-F]+ ('.' [0-9]+)?;
    vec3 = [0-9a-fA-F]+ ('.' [0-9]+)? ',' [0-9a-fA-F]+ ('.' [0-9]+)? ',' [0-9a-fA-F]+ ('.' [0-9]+)?;
    vec2 = [0-9a-fA-F]+ ('.' [0-9a-fA-F]+)? ',' [0-9a-fA-F]+ ('.' [0-9a-fA-F]+)?;

    # Integer Constants
    int_const = digit+;

    # Hexadecimal Constants
    hex_prefix = '0' [xX];
    hex_digit = [0-9a-fA-F];
    hex_const = hex_prefix hex_digit+;

    # Block controls
    block_begin = /{/i;
    block_end = /}/i;

    # Macro keyword
    macro = /macro/i;

    # Identifiers
    identifier = [a-zA-Z_][a-zA-Z0-9_]*;

    main := |*
        # End cmd
        semi => semi_tok;
        
        # Registers
        rgbaq => rgbaq_tok;
        xyz2 => xyz2_tok;
        prim => prim_tok;
        fog => fog_tok;
        fogcol => fogcol_tok;
        scissor => scissor_tok;
        signal => signal_tok;
        finish => finish_tok;

        # Modifiers
            # Primitive Types
            mod_point => mod_point_tok;
            mod_line => mod_line_tok;
            mod_linestrip => mod_linestrip_tok;
            mod_triangle => mod_triangle_tok;
            mod_trianglestrip => mod_trianglestrip_tok;
            mod_trianglefan => mod_trianglefan_tok;
            mod_sprite => mod_sprite_tok;
            # Primitive Modifiers
            mod_gouraud => mod_gouraud_tok;
            mod_fogging => mod_fogging_tok;
            mod_aa1 => mod_aa1_tok;

        # Vectors
        vec4 => vec4_tok;
        vec3 => vec3_tok;
        vec2 => vec2_tok;

        # Integer Constants
        int_const => int_const_tok;

        # Hexadecimal Constants
        hex_const => hex_const_tok;

        # Block controls
        block_begin => block_begin_tok;
        block_end => block_end_tok;

        # Macro keyword
        macro => macro_tok;

        # Identifiers
        identifier => identifier_tok;
        space;

    # Comments
    '/*' { fgoto c_comment; };
    '//' any* :>> [\n\r] @{ fgoto main; };
    *|;
}%%

%% write data;

class Scan
{
public:
    ~Scan();
    void init();
    void execute(const char* data, size_t len);

private:
    int cs;
    int act;
    const char* ts = nullptr;
    const char* te = nullptr;

    void* lparser;
};

Scan::~Scan()
{
    ParseFree(lparser, free);
}

void
Scan::init()
{
    lparser = ParseAlloc(malloc);

    %% write init;
}

void
Scan::execute(const char* data, size_t len)
{
    const char* p = data;
    const char* pe = data + len;
    const char* eof = pe;
    const char *line_start = p;
    const char* line_end = p;
    if(pByLine)
    {
by_line:
        while(line_end < eof && *line_end != '\n')
        {
            line_end++;
        }
        p = line_start;
        pe = line_end;
        line_start = line_end++;
    }

    %% write exec;

    if(!valid)
        return;
    if(pByLine)
    {
        line++;
        if(p < eof)
        {
            goto by_line;
        }
    }
    printf("Done\n");
}


void FailError(const char* ts, const char* te)
{
    logger::error("Parser error at line %d.\n\tEither the token `%s` is erroneous, or the previous token is invalid.", line, std::string(ts, te - ts).c_str());
}


void print_help(char* argv0)
{
    fmt::print("Usage: {} <file> <output> [--backend=<backend>] [--b<backend arguments>]\n\t"
            "General Arguments:\n\t"
            "  --help, -h\n\t"
            "    Prints this help message\n\t"
            "  --oneshot-parse\n\t"
            "    Parses the entire file in one go. Probably faster, but you lose proper parsing error handling\n\t"
            "Optimization settings:\n\t"
            "  --keep-deadstore\n\t"
            "    Disables dead store optimization. (Consecutive writes to stateless registers)\n\t"
            " --no-tag-prim\n\t"
            "    Disables packing the first PRIM write into the GIFTag\n\t"
            "Valid backends are:\n\t"
            "  c_code(default)\n\t"
            "    Generates a c file with an array for each gif block\n"
            "For backend specific help, please pass --bhelp to your backend\n" , argv0);
};


std::string_view file_in = "";
std::string_view file_out = "";
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        print_help(argv[0]);
        return 1;
    }
    for(int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if(arg.starts_with("--backend="))
        {
            std::string_view backend_str = arg.substr(10);
            if(backend_str == "c_code")
            {
                fmt::print("Using C backend\n");
                backend = new c_code_backend();
                if(!backend->arg_parse(argc, argv))
                {
                    fmt::print("Use --bhelp for valid backend configuration arguments\n");
                    return 1;
                }
            }
            else
            {
                fmt::print("Unknown backend: {}\n", backend_str);
                return 1;
            }
        }
        else if(arg == "--help" || arg == "-h")
        {
            print_help(argv[0]);
            return 1;
        }
        else if (arg == "--keep-deadstore")
        {
            machine.DisableOptimization(Machine::Optimization::DEAD_STORE_ELIMINATION);
        }
        else if (arg == "--no-tag-prim")
        {
            machine.DisableOptimization(Machine::Optimization::USE_TAG_PRIM);
        }
        else if (arg == "--oneshot-parse")
        {
            pByLine = false;
        }
        else if(file_in.empty() && !arg.starts_with("-"))
        {
            file_in = arg;
        }
        else if (file_out.empty() && !arg.starts_with("-"))
        {
            file_out = arg;
        }
        else if(!arg.starts_with("--b"))
        {
            fmt::print("Unknown argument: {}\n", arg);
            return 1;
        }
    }

    if(backend == nullptr)
    {
        logger::info("No backend specified, using default 'c_code'");
        backend = new c_code_backend();
        if(!backend->arg_parse(argc, argv))
        {
            fmt::print("Use --bhelp for valid backend configuration arguments\n");
            return 1;
        }
    }

    machine.SetBackend(backend);

    backend->set_output(file_out);

    if(file_in.empty())
    {
        fmt::print("No input file specified\n");
        return 1;
    }

    if(file_out.empty())
    {
        fmt::print("No output file specified. Printing to stdout\n");
    }

    char buffer[8192];
    FILE* fin;
    Scan scan;
    long numbytes;

    //Read the whole file into the buffer.
    fin = fopen(file_in.cbegin(), "r");
    if(fin == nullptr)
    {
        fmt::print("Failed to open file: {}\n", file_in);
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    numbytes = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if(numbytes > sizeof(buffer))
    {
        fmt::print("File {} bytes is too large for internal buffer {} bytes :(\n", numbytes, sizeof(buffer));
        return 1;
    }

    fread(buffer, 1, numbytes, fin);

    //Parse the buffer in one fell swoop.
    scan.init();
    scan.execute(buffer, numbytes);
    delete backend;
    fclose(fin);
    return 0;
}
