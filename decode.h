#ifndef DECODE_H
#define DECODE_H
#include<stdio.h>
#include "types.h" // Contains user defined types


/* 
 * Structure to store information required for
 * decoding received encoded image to secret text file
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Stego Image info */
    char *src_stego_image_fname ;
    FILE *fptr_src_stego_image;
    long int postion_to_skip;  //Used to skip the secret file extension and jump to secret message length
    char dec_image_char;       //Store the character while debugging the message or magic string
    char image_dec_data[MAX_IMAGE_BUF_SIZE];

    /* Output Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];  //to store the encoded extension length of secret file
    char image_secret_data[MAX_IMAGE_BUF_SIZE]; 
    long length_secret_msg;  //Length of the secret text is stored
   
} DecodeInfo;

/*Validation of the input file format*/
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/*Validate open files*/
Status open_files_decode(DecodeInfo *decInfo);

/*BMP file size identifier*/
uint get_image_size_for_bmp_decode(FILE *fptr_image);


/*Decode string from image and check the magic string*/
Status decode_magic_data_from_image(char *magic_str, int size, DecodeInfo *decInfo);

/*Decode single character from image*/
Status decode_image_character(DecodeInfo *decInfo, char *ch);

/*Decode length of the extension of the secret file*/
Status decode_extn_length(DecodeInfo *decInfo);

/*Decode length of secret text*/
Status decode_length_secret_text (DecodeInfo *decInfo);

/*Decode length of the string*/
Status decode_length_string(DecodeInfo *decInfo, int size, char *ch, long int *output);

/*Decode the secret message*/
Status decode_secret_message(DecodeInfo *decInfo);

/*Decoding main function*/
Status do_decoding(DecodeInfo *decInfo);


#endif