#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Checking for input bmp file
    if(argv[2] != NULL && (strcmp(strstr(argv[2],"."), ".bmp")==0))
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    //Checking for input secret code file
    if(argv[3] != NULL && (strcmp(strstr(argv[3],"."), ".txt")==0))
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    //Checking for output file
    if(argv[4] != NULL && (strcmp(strstr(argv[4],"."), ".bmp")==0))
    {
        printf("Enter 3");
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;
}

uint get_file_size(FILE *secret)
{
    fseek(secret,0,SEEK_END);
    return ftell(secret);
}

Status check_capacity(EncodeInfo *encInfo)
{
    //get bmp file size
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);

    //get secret text file size
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    //Check the image file capacity is sufficient to hold the secret message
    //54 - Header :: 2 - Magic string :: 4 - length of secret file extension :: 
    //4 - char of extension of the secret file :: 4 size of the secret message 
    if(encInfo -> image_capacity > 54 + ((2 + 4 + 4 + 4 + encInfo->size_secret_file)*8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src, FILE *fptr_dest)
{
    char header[54];
    //bring back the file pointer to first position in bmp
    fseek(fptr_src, 0, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src);
    fwrite(header, sizeof(char), 54, fptr_dest);
    return e_success;
}

Status encode_byte_to_lsb(char data, char* data_arr, int val)
{
    unsigned char mask = 1;    
    for(int i=0; i < val; i++)
    {
        data_arr [i] = (data_arr[i] & 0xFE) | (data >> (val-1-i) & mask);        
        
    }
    return e_success;
}

Status encode_size_to_lsb(int data, char* data_arr, int val)
{
    
    unsigned char mask = 1;
    
    for(int i=0; i < val; i++)
    {
        data_arr [i] = (data_arr[i] & 0xFE) | (data >> (val-1-i) & mask); 
    }
    return e_success;
}

Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo, int val)
{
    for(int i=0; i<size; i++)
    {        
        fread(encInfo->image_data, val, sizeof(char), encInfo->fptr_src_image);        
        encode_byte_to_lsb(data[i], encInfo->image_data, val);
        fwrite(encInfo->image_data, val, sizeof(char),encInfo->fptr_stego_image);        
    }
    return e_success;
}

Status encode_size_to_image(int data, EncodeInfo *encInfo, int val)
{
    char str[val];
    fread(str, val, sizeof(char), encInfo->fptr_src_image);        
    encode_size_to_lsb(data, str, val);
    fwrite(str, val, sizeof(char),encInfo->fptr_stego_image);       
    
    return e_success;
}

Status encode_magic_string(char * magic_str, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_str, strlen(magic_str), encInfo, 8);
    return e_success;
}

Status encode_secret_file_extn(char *file_ext, EncodeInfo *encInfo)
{
    file_ext = ".txt";   
    encode_data_to_image(file_ext, strlen(file_ext), encInfo, 8);   
    return e_success;
}

Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, 32, sizeof(char), encInfo->fptr_src_image);   
    encode_size_to_lsb(file_size, str, 32);
    fwrite(str, 32, sizeof(char),encInfo->fptr_stego_image);        
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    fseek(encInfo->fptr_secret, 0, SEEK_SET); //to bring the cursor to first position
    for(int i=0; i<encInfo->size_secret_file; i++)
    {
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        fread(&ch, 1, sizeof(char), encInfo->fptr_secret);
        encode_byte_to_lsb(ch, encInfo->image_data, 8);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    char ch;
    while(fread(&ch, 1, 1, encInfo->fptr_src_image)>0)
    {
        fwrite(&ch, 1, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //Check whether the file is openable or not
    if(open_files(encInfo) == e_success)
    {
        printf("Opened all the files successfully.\n"); 

        //Check the capacity of the image to code the secret message
        if(check_capacity(encInfo) == e_success)
        {
            printf("Possible to encode the data.  Encoding is in progress.\n");
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
            {
                printf("Copied Header successfully.\n");
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Magic string encoded successfully.\n");
                    int length_extension = strlen(strstr(encInfo->secret_fname,".")); 
                    if(encode_size_to_image(length_extension, encInfo, sizeof(int)*8) == e_success)
                    {                       
                        printf("Extension size of the secret file is encoded successfully.\n");
                        
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)== e_success)
                        {
                            printf("Secret file extension encoded successfully.\n");
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
                            {
                                printf("Secret file size is encoded successfully.\n");
                                if(encode_secret_file_data(encInfo)== e_success)
                                {
                                    printf("Secret data encoded successfully.\n");
                                    if(copy_remaining_img_data(encInfo) == e_success)
                                    {
                                        printf("All data encoded successfully.  Encoding completed successfully.\nFile ready to send.\n");
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf("Failure to encode the remaining data.\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode secret data.\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode the size of the secret file.\n");
                                return e_success;
                            }

                        }
                        else
                        {
                            printf("Failed to encode secret file extension");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to copy the extension size of the secret file.\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to copy the magic string.\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy the header.\n");
                return e_failure;
            }
        }
        else
        {
            printf("Error: Image capacity is not sufficient to encode.\n");
            return e_failure;
        }  
        return e_success;       
    }
    else
    {
        printf("Error in opening the file.\n");        
    }
}
