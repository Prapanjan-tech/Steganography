#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Stego Image file
 * Output: Secret Text file
 * Return Value: e_success or e_failure, on file errors
 */

//Function to validate the opening of the file
Status open_files_decode(DecodeInfo *decInfo)
{
    // Src stego Image file
    decInfo->fptr_src_stego_image = fopen(decInfo->src_stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_src_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_stego_image_fname);

    	return e_failure;
    }
    printf("BMP file opened successfully for reading the image.\n");    

    // Secret file
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    // Do Error handling
    if (decInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

    	return e_failure;
    }
    printf("Text file is opened with write access to write the secret data.\n");  
    // No failure return e_success
    return e_success;
}

//Function to validate the user interface information on files
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Checking for input bmp file
    if(argv[2] != NULL && (strcmp(strstr(argv[2],"."), ".bmp")==0))
    {
        decInfo -> src_stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }    

    //Checking for output file
    if(argv[3] != NULL && (strcmp(strstr(argv[3],"."), ".txt")==0))
    {
        decInfo -> secret_fname = argv[3];
    }
    else
    {
        
        decInfo -> secret_fname = "secret_decode.txt";
        if(argv[3] != NULL)  //If the provided output file format is not compatible
        {
            printf("The secret message can't be stored in the provided format.  Hence the message is stored in the text file: %s\n", decInfo->secret_fname);
        }
    }
    return e_success;
}

//Common function to find the encoded character
Status decode_image_character(DecodeInfo *decInfo, char *ch)
{
    decInfo->dec_image_char =0;
    int mask = 1;
    for(int i=0; i<8; i++)
    {
        int temp = ch[i]  & mask;
        decInfo->dec_image_char = (temp << (7-i)) | decInfo->dec_image_char;
    }
    
    return e_success;
}

//Function decode magic string and find the secret message encoded file
Status decode_magic_data_from_image(char *magic_str, int size, DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_src_stego_image, 54, SEEK_SET);  //Skip the first 54 header 
    for(int i=0; i<size; i++)
    {
        fread(decInfo->image_dec_data, 8, sizeof(char), decInfo->fptr_src_stego_image);
        decode_image_character(decInfo,decInfo->image_dec_data);
        if(decInfo->dec_image_char != magic_str[i]) //Checks the magic character by character.  If anything not equals then it will return failue
        {
           return e_failure;
        }
    }
    return e_success;
}


//common function to find the length of the string
Status decode_length_string(DecodeInfo *decInfo, int size, char *ch, long int *output)
{
    *output =0;  
    int mask = 1;
    for(int i=0; i<size; i++)
    {
        int temp = ch[i]  & mask;
        *output = (temp << (size-1-i)) | *output;
    }
    return e_success;
}

//Function to find the length of the encoded secret file extension
Status decode_extn_length(DecodeInfo *decInfo)
{
    
    fread(decInfo->extn_secret_file, 32, sizeof(char), decInfo->fptr_src_stego_image);
    decode_length_string(decInfo, 32, decInfo->extn_secret_file, &decInfo->postion_to_skip);
    return e_success;
}

//Function to find the length of the secret message
Status decode_length_secret_text (DecodeInfo *decInfo)
{
    char ch[32];
    fread(ch, 32, sizeof(char), decInfo->fptr_src_stego_image);
    decode_length_string(decInfo, 32, ch, &decInfo->length_secret_msg);
    return e_success;
}


//Function to decode the secret message 
Status decode_secret_message(DecodeInfo *decInfo)
{
    
    for(int i=0; i<decInfo->length_secret_msg; i++)
    {
        fread(decInfo->image_secret_data, 8, sizeof(char), decInfo->fptr_src_stego_image);
        decode_image_character(decInfo,decInfo->image_secret_data);
        fputc(decInfo->dec_image_char, decInfo->fptr_secret);                
    }
    return e_success;
}

//Main function for the decoding process
Status do_decoding(DecodeInfo *decInfo)
{
    //Check whether the file is openable or not
    if(open_files_decode(decInfo)== e_success)
    {
        printf("Opened all files successfully.\n");
        //Check the magic string mataches or not
        if(decode_magic_data_from_image(MAGIC_STRING, strlen(MAGIC_STRING),decInfo)== e_success)
        {
            printf("Magic string decoded and the secret encoded BMP file is found.  Proceeding to decoding.\n");
            if(decode_extn_length(decInfo) == e_success)
            {
                printf("Extension size is decoded successfully.\n");
                int new_post_after_skipping_extn = ftell(decInfo->fptr_src_stego_image)+(decInfo->postion_to_skip*8);  //new position after skipping the extension string of the encoded secret file
                fseek(decInfo->fptr_src_stego_image, new_post_after_skipping_extn, SEEK_SET);  // Skipped the extension text of the secret file
                if(decode_length_secret_text(decInfo) == e_success)
                {
                    printf("Length of the secret message is decoded successfully.\n");
                    if(decode_secret_message(decInfo) == e_success)
                    {
                        printf("Successfully decoded the secret message.\n");
                        return e_success;
                    }
                    else
                    {
                        printf("Failed to decode the secret message.\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to decode the length of the secret message.\n");
                    return e_failure;
                }
            }
            else
            {                
                printf("Failed to decode size of the extension.\n");
                return e_failure;
            }
        }
        else
        {
            printf("Magic string is not found or failed to decode.\n");
            return e_failure;
        }
    }
    else
    {
        printf("Error in opening the file\n.");
        return e_failure;
    }
    return e_success;
}
