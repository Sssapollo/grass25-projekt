#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

struct Request {
    uint32_t addr;
    uint32_t data;      
    uint8_t  r;         
    uint8_t  w;        
    uint32_t fault;    
    uint8_t  faultBit; 
};

struct Result {
    uint32_t cycles;
    uint32_t errors;
};

extern struct Result run_simulation(
                          uint32_t    cycles,
                          const char *tracefile,
                          uint8_t     endianness,
                          uint32_t    lat_scramble,
                          uint32_t    lat_encrypt,
                          uint32_t    lat_mem,
                          uint32_t    seed,
                          uint32_t    numRequests,
                          struct Request *requests
    );
const char* usage_msg = 
    "Usage: %s [options] <file>\n   "
    "Options:\n"
    "   -c,--cycles N: uint32_t        Number of cycles (default: 1)\n"
    "   -t,--tf : String               Path to the TraceFile\n"
    "   -e, --endianness N:uint8_t            Endianness: 0=Little-Endian, 1=Big-Endian(default:0)\n"
    "   -s, --latency-scrambling N:uint32_t    Latency for Address Scrambling in clock cycles(default:0)\n"
    "   -n, --latency-encrypt N:uint32_t       Latency for Encryption/Decryption in clock cycles(default:0)\n"
    "   -m, --latency-memory-access N:uint32_t Latency for Memory Access in clock cycles(default:0)\n"
    "   -d, --seed N:uint32_t                  Seed for the pRNG\n"
    "   -h,--help: flag                Show the help message\n";

const char* help_msg =
    "Positional arguments:\n"
    "  <file>  The path to input file:list of requests\n"
    "\n"
    "Optional arguments:\n"
    "  -c, --cycles N                Number of cycles (uint32_t, default: 1)\n"
    "  -t, --tf PATH                 Path to the TraceFile (string)\n"
    "  -e, --endianness N            Endianness (uint8_t): 0=Little-Endian, 1=Big-Endian\n"
    "  -s, --latency-scrambling N    Latency for Address Scrambling (uint32_t)\n"
    "  -n, --latency-encrypt N       Latency for Encryption/Decryption (uint32_t)\n"
    "  -m, --latency-memory-access N Latency for Memory Access (uint32_t)\n"
    "  -d, --seed N                  Seed for pRNG (uint32_t)\n"
    "  -h, --help                    Show this help message\n";
void print_usage(const char* progname) {
    fprintf(stderr, usage_msg, progname);
}

void print_help(const char* progname) {
    printf(usage_msg, progname);
    printf("\n%s", help_msg);
}


int parse_cli(int argc, char *argv[], uint32_t *cycles, char **tracefile, char **input_file,
              uint8_t *endianness, uint32_t *latency_scrambling, uint32_t *latency_encrypt,
              uint32_t *latency_memory_access, uint32_t *seed) {  
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"tf", required_argument, 0, 't'},
        {"endianness", required_argument, 0, 'e'},
        {"latency-scrambling", required_argument, 0, 's'},
        {"latency-encrypt", required_argument, 0, 'n'},
        {"latency-memory-access", required_argument, 0, 'm'},
        {"seed", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int opt;
    while ((opt = getopt_long(argc,argv,"c:t:e:s:n:m:d:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                *cycles = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 't':
                *tracefile = optarg;
                break;
            case 'e':
                *endianness = (uint8_t)strtoul(optarg, NULL, 10);
                if (*endianness != 0 && *endianness != 1) {
                    fprintf(stderr, "Invalid endianness value. Use 0 for Little-Endian or 1 for Big-Endian.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                *latency_scrambling = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'n':
                *latency_encrypt = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'm':
                *latency_memory_access = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'd':
                *seed = (uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "No input file specified.\n");
        return 1;
    }
     while (optind < argc) {
        if (*input_file != NULL) {
            fprintf(stderr, "Too many input files specified.\n");
            return 1;
        }
        *input_file = argv[optind];
        optind++;
    }

    return EXIT_SUCCESS;
}

//parse uint32_t argument
static uint32_t parse_uint32(const char *inputString) {
    if (!inputString || *inputString == '\0') return 0;
    errno = 0;
    char *endptr = NULL;
    uint64_t val;
    if (inputString[0] == '0' && (inputString[1] == 'x' || inputString[1] == 'X')) {
        val = strtoull(inputString + 2, &endptr, 16);
    } else {
        val = strtoull(inputString, &endptr, 10);    
    }
    if (*endptr != '\0' || errno == ERANGE || val > UINT32_MAX) return -1;
    return (uint32_t)val;
}


//parse uint8_t argument
static uint8_t parse_uint8(const char *inputString) {
    if (!inputString || *inputString == '\0') return 0;
    errno = 0;
    char *endptr = NULL;
    uint64_t val;
    val = strtoull(inputString, &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE || val > UINT8_MAX) return -1;
    return (uint8_t)val;
}

//extract argument from a line
int extract_arg(const char* line,int counter, char* arg_buf) {
    int temp = 0;
    while (line[counter] != '\0' && line[counter] != ',') {
        if (line[counter] != ' ' && line[counter] != '\t') {
            arg_buf[temp++] = line[counter];
        }
        counter++;
    }
    arg_buf[temp] = '\0';
    if (line[counter] == ',') counter++;
    return counter;
}



const char* split_next_line(const char* code, char* type, char* address, char* data, char* fault, char* faultbit) {
    type[0] = address[0] = data[0] = fault[0] = faultbit[0] = '\0';
    if (!code || *code == '\0') return NULL;
    while (*code == '\n'||*code == '\t'||*code == ' ') code++;
    if (*code == '\0') return NULL;
    char line[256];
    
    size_t len = 0;
    while (code[len] && code[len] != '\n' && len < 255 && code[len] != '\0') {
        line[len] = code[len];
        len++;
    }
    line[len] = '\0';


    int temp = 0;
    while (line[temp] != '\0' && line[temp] != ' ' && line[temp] != '\t') {
        type[temp] = line[temp];
        temp++;
        if (line[temp] == ',')break;
    }
    type[temp] = '\0';
    int counter = temp + 1;// Skip the comma 
    counter = extract_arg(line, counter, address);
    counter = extract_arg(line, counter, data);
    counter = extract_arg(line, counter, fault);
    counter = extract_arg(line, counter, faultbit);



    if (code[len] == '\n') {
        return code + len + 1;
    }
    else {
        return code + len;
    }
}




int parse_requests(const char *input, uint32_t *numRequests, struct Request **requests) {
    // TODO: Implement request parsing logic
    uint32_t count = 0;
    uint32_t capacity = 256; // our default capacity for requests
    struct Request *arr = malloc(capacity * sizeof(struct Request));
     if (!arr) {
        perror("malloc");
        return -1;
    }


    char type_buf[32], addr_buf[64], data_buf[64], fault_buf[64], faultbit_buf[16];

    const char *new_line = input;

    while(*new_line == '\n' || *new_line == ' '|| *new_line == '\t') {
        new_line++; 
    }// Skip leading whitespace characters
    
    while (*new_line!= '\n') {
        if(*new_line == '\0') {
            break; 
        }
        new_line++;   
    }// Skip the first line as it is Header

    
    while ((new_line = split_next_line(new_line,
                                     type_buf, addr_buf, data_buf,
                                     fault_buf, faultbit_buf))) {

       
        if (type_buf[0] == '\0') continue;

        
        struct Request req = {0};
        req.fault = UINT32_MAX;
        req.faultBit = 0;
        req.r = 0;
        req.w = 0;
        req.addr = 0;
        req.data = 0;


        char t = (char)tolower((unsigned char)type_buf[0]);

        if (t == 'r') {
            req.r = 1;
            req.addr = parse_uint32(addr_buf);
            if (req.addr == (uint32_t)-1) {
                fprintf(stderr, "Invalid address in read request: %s\n", addr_buf);
                free(arr);
                return -1;
            }
            

        } else if (t == 'w') {
            req.w = 1;
            req.addr = parse_uint32(addr_buf);
            if (req.addr == (uint32_t)-1) {
                fprintf(stderr, "Invalid address in write request: %s\n", addr_buf);
                free(arr);
                return -1;
            }
            req.data = parse_uint32(data_buf);
            if (req.data == (uint32_t)-1) {
                fprintf(stderr, "Invalid data in write request: %s\n", data_buf);
                free(arr);
                return -1;
            }
        } else if (t == 'f') {
             if(addr_buf[0] != '\0' || data_buf[0] != '\0') {
                fprintf(stderr, "Fault request should not have address or data: %s %s\n", addr_buf, data_buf);
                free(arr);
                return -1;
            }
            req.fault    = parse_uint32(fault_buf);
            if (req.fault == (uint32_t)-1) {
                fprintf(stderr, "Invalid fault in fault request: %s\n", fault_buf);
                free(arr);
                return -1;
            }
            req.faultBit = parse_uint8(faultbit_buf);
            if (req.faultBit == (uint8_t)-1) {
                fprintf(stderr, "Invalid fault bit in fault request: %s\n", faultbit_buf);
                free(arr);
                return -1;
            }
           
        }

        if (t != 'f' && (fault_buf[0] || faultbit_buf[0])) {
            req.fault    = parse_uint32(fault_buf);
            if (req.fault == (uint32_t)-1) {
                fprintf(stderr, "Invalid fault in request: %s\n", fault_buf);
                free(arr);
                return -1;
            }
            req.faultBit = parse_uint8(faultbit_buf);
            if (req.faultBit == (uint8_t)-1|| (req.faultBit > 8)) {
                fprintf(stderr, "Invalid fault bit in request: %s\n", faultbit_buf);
                free(arr);
                return -1;
            }
        }




        arr[count++] = req;
    } 

    *numRequests = count;
    *requests    = arr;
    
    return 0;
}


FILE* validate_and_open_read(const char *path) {
    FILE* file;
    if (!(file = fopen(path, "r"))) {
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fileno(file), &statbuf)) {
        fclose(file);
        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fclose(file);
        return NULL;
    }
    return file;
}


int main(int argc, char** argv){

    static uint32_t cycles = 1000;
    static char *tracefile = NULL;
    static char *input_file = NULL;
    static uint8_t endianness = 0; // Default: Little-Endian
    static uint32_t latency_scrambling = 0; // Default: 0
    static uint32_t latency_encrypt = 0; // Default: 0
    static uint32_t latency_memory_access = 0; // Default: 0
    static uint32_t seed = 1; // Default: 1

    int result = parse_cli(argc, argv, &cycles , &tracefile, &input_file,
                           &endianness, &latency_scrambling, &latency_encrypt,
                           &latency_memory_access, &seed);
    if (result != 0 || input_file == NULL) {
        return result;
    }


     FILE* file = validate_and_open_read(input_file);
    if (file == NULL) {
        fprintf(stderr, "Failed to open input file '%s'.\n", input_file);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* input = (char*)malloc(file_size + 1);
    if (input == NULL) {
        fprintf(stderr, "Failed to allocate memory for input file.\n");
        fclose(file);
        return 1;
    }
    fread(input, 1, file_size, file);
    fclose(file);
    input[file_size] = '\0';
    // TODO:Parse the input file to get the number of requests and their details
    uint32_t numRequests = 0;
    struct Request *requests = NULL;

    if (parse_requests(input, &numRequests, &requests) != 0) {
        fprintf(stderr, "Failed to parse requests from input file.\n");
        free(input);
        return 1;
    }

    struct Result res = run_simulation(
                   cycles,
                   tracefile,
                   endianness,
                   latency_scrambling,
                   latency_encrypt,
                   latency_memory_access,
                   seed,
                   numRequests,
                   requests);



    printf("Simulation completed:\n");
    printf("Cycles: %u\n", res.cycles);
    printf("Errors: %u\n", res.errors);


    free(input);
    free(requests);
    return 0;
}