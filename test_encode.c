#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int args, char *argv[])
{
    if(check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and Validate information are successful\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("Completed the encoding.\n");
            }
            else
            {
                printf("Failed to encode the data.\n");
            }
        }
        else
        {
            printf("Error: Failed to validate encode arguments...\n");
        }
    }

    else if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        printf("Selected Decoding\n");
        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and Validate information are successful\n");
            if(do_decoding(&decInfo) == e_success)
            {
                printf("Completed the Decoding.\n");
            }
            else
            {
                printf("Failed to Decode the data.\n");
            }
        }
        else
        {
            printf("Error: Failed to validate encode arguments...\n");
        }
    }
    else
    {
        printf("Invalid Operation selected\nEncoding: ./a.out -e beautiful.bmp secret.text\nDecoding: ./a.out -d stego.bmp\n");
    }
    
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e")==0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1],"-d")==0)
    {
        return e_decode;
    }
    else    
    {
        return e_unsupported;
    }
}
