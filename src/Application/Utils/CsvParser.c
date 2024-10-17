//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		CsvParser.h
@brief      File containing the defination of CSV parser APIs
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>

/* Application Includes */
#include "DebugLog.h"
#include "CsvParser.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void destroy_field(CSV_FIELD *field);
//-------------------------------------------------------------------------------------------------
static INT32 read_next_field(FILE *fp, CHAR field_delim, CHAR text_delim, CSV_FIELD *field);
//-------------------------------------------------------------------------------------------------
static INT32 append_row(CSV_BUFFER *buffer);
//-------------------------------------------------------------------------------------------------
static INT32 append_field(CSV_BUFFER *buffer, size_t row);
//-------------------------------------------------------------------------------------------------
static INT32 set_field(CSV_FIELD *field, CHAR *text);
//-------------------------------------------------------------------------------------------------
static INT32 copy_row(CSV_BUFFER *dest, size_t dest_row, CSV_BUFFER *source, size_t source_row);
//-------------------------------------------------------------------------------------------------
static INT32 clear_row(CSV_BUFFER *buffer, size_t row);
//-------------------------------------------------------------------------------------------------
static INT32 copy_field(CSV_BUFFER *dest, INT32 dest_row, INT32 dest_entry, CSV_BUFFER *source, INT32 source_row, INT32 source_entry);
//-------------------------------------------------------------------------------------------------
static INT32 clear_field(CSV_BUFFER *buffer, size_t row, size_t entry);
//-------------------------------------------------------------------------------------------------
static void append_row_in_file(CHARPTR fileName);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateCsvBuffer
 * @return  Allocated CSV buffer
 */
CSV_BUFFER *CreateCsvBuffer(void)
{
    CSV_BUFFER *buffer = malloc(sizeof(CSV_BUFFER));
    if (buffer == NULL)
    {
        return NULL;
    }

    buffer->field = NULL;
    buffer->rows = 0;
    buffer->width = NULL;
    buffer->field_delim = ',';
    buffer->text_delim = '"';
    return buffer;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DestroyCsvBuffer
 * @param   buffer
 */
void DestroyCsvBuffer(CSV_BUFFER *buffer)
{
    size_t i, j;

    if(buffer == NULL)
    {
        EPRINT(UTILS, "buffer ptr null");
        return;
    }

    for (i = 0; i < buffer->rows; i++)
    {
        for (j = 0; j < buffer->width[i]; j++)
        {
            destroy_field(buffer->field[i][j]);
        }

        FREE_MEMORY(buffer->field[i]);
    }

    FREE_MEMORY(buffer->field);
    FREE_MEMORY(buffer->width);
    FREE_MEMORY(buffer);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   LoadCsvFile
 * @param   buffer
 * @param   file_name
 * @return  return 0 on success else return non zero on failure
 */
INT32 LoadCsvFile(CSV_BUFFER *buffer, CHAR *file_name)
{
    if (buffer == NULL)
    {
         EPRINT(UTILS, "buffer ptr null");
         return 1;
    }

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        return 1;
    }

    INT32 next = 1;
    BOOL end = FALSE;
    BOOL first = TRUE;
    INT32 i = -1, j = -1;

    // loop until it is not end
    while (!end)
    {   // skip read field if first time
        if (!first)
        {
            next = read_next_field(fp, buffer->field_delim, buffer->text_delim, buffer->field[i][j-1]);
        }

        if (next == 2)
        {
            end = TRUE;
        }

        if (next == 1)
        {
            if (append_row(buffer) != 0)
            {
                fclose(fp);
                return 2;
            }

            j = 1;
            i++;
        }

        if (next == 0)
        {
            if (append_field(buffer, i) != 0)
            {
                fclose(fp);
                return 2;
            }
            j++;
        }

        if (first)
        {
            first = FALSE;
        }
    }

    fclose(fp);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   SaveCsvFile
 * @param   file_name
 * @param   buffer
 * @return  return 0 on success else return non zero on failure
 */
INT32 SaveCsvFile(CHAR *file_name, CSV_BUFFER *buffer)
{
    size_t  i, j, k;
    CHAR    *chloc;

    if(buffer == NULL)
    {
        EPRINT(UTILS, "buffer ptr null");
        return 1;
    }

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        return 1;
    }

    CHAR text_delim = buffer->text_delim;
    CHAR field_delim = buffer->field_delim;

    for(i = 0; i < buffer->rows; i++)
    {
        for(j = 0; j < buffer->width[i]; j++)
        {
            chloc = strchr(buffer->field[i][j]->text, text_delim);
            if(chloc == NULL)
            {
                chloc = strchr(buffer->field[i][j]->text, field_delim);
            }

            if(chloc == NULL)
            {
                chloc = strchr(buffer->field[i][j]->text, '\n');
            }

            /* if any of the above characters are found, chloc will be set and we must use text deliminators. */
            if(chloc != NULL)
            {
                fputc(text_delim, fp);
                for(k = 0; k < buffer->field[i][j]->length - 1; k++)
                {
                    /* if there are any text delims in the string, we must escape them. */
                    if(buffer->field[i][j]->text[k] == text_delim)
                    {
                            fputc(text_delim, fp);
                    }

                    fputc(buffer->field[i][j]->text[k], fp);
                }

                fputc(text_delim, fp);
                chloc = NULL;
            }
            else
            {
                fputs(buffer->field[i][j]->text, fp);
            }

            if(j < buffer->width[i] - 1)
            {
                fputc(field_delim, fp);
            }
            else if (i < buffer->rows - 1)
            {
                fputc('\n', fp);
            }
        }
    }

    fclose(fp);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCsvField
 * @param   dest
 * @param   dest_len
 * @param   src
 * @param   row
 * @param   entry
 * @return  return 0 on success else return non zero on failure
 */
INT32 GetCsvField(CHAR *dest, size_t dest_len, CSV_BUFFER *src, size_t row, size_t entry)
{
    size_t i;

    if(dest == NULL)
    {
        EPRINT(UTILS, "buffer ptr null");
        return -1;
    }

    if (dest_len == 0)
    {
        return 3;
    }

    /* row does not exist OR entry does not exist */
    if ((row >= src->rows) || (entry >= src->width[row]))
    {
        for (i = 0; i < dest_len; i++)
        {
                dest[0] = '\0';
        }

        /* If the requested entry does not exist or is invalid, we clear the string provided consistent with the case of an empty entry. */
        return 2;
    }
    else
    {
        /* If destination is not large enough to hold the whole entry, strncpy will truncate it for us. */
        strncpy(dest, src->field[row][entry]->text, dest_len);
        dest[dest_len] = '\0';
    }

    if (src->field[row][entry]->length > dest_len + 1)
    {
        return 1;
    }

    if (src->field[row][entry]->length == 0)
    {
        return 2;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   SetCsvField
 * @param   buffer
 * @param   row
 * @param   entry
 * @param   field
 * @return  return 0 on success else return non zero on failure
 */
INT32 SetCsvField(CSV_BUFFER *buffer, size_t row, size_t entry, CHAR *field)
{
    while (row >= buffer->rows)
    {
        append_row(buffer);
    }

    while (entry >= buffer->width[row])
    {
        append_field(buffer, row);
    }

    return set_field(buffer->field[row][entry], field);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UpdateLanguageFiles
 */
void UpdateLanguageFiles(void)
{
    struct dirent *entry;

    DIR *dir = opendir(LANGUAGES_DIR_PATH);
    if (dir == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open dir: [path=%s], [err=%s]", LANGUAGES_DIR_PATH, STR_ERR);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name, SAMPLE_CSV_FILE_NAME) != 0) && (strncmp(entry->d_name, ".", 1) != 0) && (strncmp(entry->d_name, "..", 2) != 0))
        {
            if(strstr(entry->d_name, ".csv") != NULL)
            {
                append_row_in_file(entry->d_name);
            }
        }
    }

    closedir(dir);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   NotifyLanguageUpdate
 * @return  Returns SUCCESS/FAIL
 * @note    Create a temporary file to notify configuration restore on system restart.
 */
BOOL NotifyLanguageUpdate(void)
{
    FILE *fp;

    // this file is created to notify restore config on system restart
    fp = fopen(LANGUAGE_UPDATE_NOTIFY, "w");
    if(fp == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open file: [err=%s]", STR_ERR);
        return FAIL;
    }

    fclose(fp);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateTsFile
 * @param   buffer
 * @param   fileName
 */
void CreateTsFile(CSV_BUFFER *buffer, CHARPTR fileName)
{
    FILE    *fpts;
    CHARPTR subStr = NULL;
    UINT64  row = 0;
    CHAR    buff[PATH_MAX] = "";
    CHAR    fileNameWoExt[MAX_LANGUAGE_FILE_NAME_LEN] = "";

    subStr = strrchr(fileName, '.');
    if (subStr == NULL)
    {
        EPRINT(NETWORK_MANAGER, "invld file name found: [name=%s]", fileName);
        return;
    }

    *subStr = '\0';
    snprintf(fileNameWoExt, MAX_LANGUAGE_FILE_NAME_LEN, "%s", fileName);
    snprintf(fileName, MAX_LANGUAGE_FILE_NAME_LEN, "%s.ts", fileNameWoExt);
    remove(fileName);

    fpts = fopen(fileName, "w");
    if(fpts == NULL)
    {
        EPRINT(NETWORK_MANAGER, "failed to open file: [name=%s]", fileName);
        return;
    }

    fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n<TS version=\"2.0\" language=\"es\" sourcelanguage=\"en_GB\">\n\t<context>\n\t\t<name>Matrix_NVR-X</name>\n", fpts);

    for(row = 1; row < buffer->rows; row++)
    {
        size_t translated_string_size = MAX_LABEL_STRING_SIZE;
        size_t label_string_size = MAX_LABEL_STRING_SIZE;

        CHAR *translated_string = malloc(translated_string_size + 1);
        if (translated_string == NULL)
        {
            continue;
        }

        CHAR *label_string = malloc(label_string_size + 1);
        if (label_string == NULL)
        {
            free(translated_string);
            continue;
        }

        GetCsvField(translated_string, translated_string_size, buffer, row, 1);
        GetCsvField(label_string, label_string_size, buffer, row, 0);
        if((translated_string[0] == '\r') || (translated_string[0] == '\n'))
        {
            translated_string[0] = '\0';
        }

        snprintf(buff, sizeof(buff), "\t\t<message>\n\t\t\t<source>%s</source>\n\t\t\t<translation>%s</translation>\n\t\t</message>\n", label_string, translated_string);
        fputs(buff, fpts);

        free(label_string);
        free(translated_string);
    }

    fputs("\t</context>\n</TS>", fpts);
    fclose(fpts);

    // After Generating new .ts file taken care of deleting old .qm file
    snprintf(fileName, MAX_LANGUAGE_FILE_NAME_LEN, "%s.qm", fileNameWoExt);
    remove(fileName);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   add_char
 * @param   string
 * @param   c
 * @param   ch
 * @return  return 0 on success else return non zero on failure
 */
static INT32 add_char(CHAR **string, INT32 *c, CHAR ch)
{
    CHAR *tmp = NULL;

    (*c)++;
    tmp = realloc(*string, (*c)+1);
    if (tmp == NULL)
    {
        return 1;
    }

    *string = tmp;
    (*string)[(*c)-1] = ch;
    (*string)[*c] = '\0';
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   set_field
 * @param   field
 * @param   text
 * @return  return 0 on success else return non zero on failure
 */
static INT32 set_field(CSV_FIELD *field, CHAR *text)
{
    CHAR *tmp;

    field->length = strlen(text) + 1;

    // PARASOFT : Release of memory for "field" has been taken care in "DestroyCsvBuffer" function
    tmp = realloc(field->text, field->length);
    if (tmp == NULL)
    {
        return 1;
    }

    field->text = tmp;
    snprintf(field->text, field->length, "%s", text);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   create_field
 * @return  return 0 on success else return non zero on failure
 */
static CSV_FIELD *create_field(void)
{
    // PARASOFT : Release of memory for "field" has been taken care in "DestroyCsvBuffer" function
    CSV_FIELD *field = malloc(sizeof(CSV_FIELD));
    if(field == NULL)
    {
        return NULL;
    }

    field->length = 0;
    field->text = NULL;
    set_field(field, "");
    return field;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   destroy_field
 * @param   field
 */
static void destroy_field(CSV_FIELD *field)
{
    FREE_MEMORY(field->text);
    FREE_MEMORY(field);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   read_next_field
 * @param   fp
 * @param   field_delim
 * @param   text_delim
 * @param   field
 * @return  return 0 on success else return non zero on failure
 */
static INT32 read_next_field(FILE *fp, CHAR field_delim, CHAR text_delim, CSV_FIELD *field)
{
    INT32   ch = 'a';
    BOOL    done = FALSE;
    BOOL    in_text = FALSE;
    BOOL    esc = FALSE;
    INT32   c = 0;
    CHAR    *tmp = malloc(1);

    if(tmp == NULL)
    {
        EPRINT(UTILS, "fail to allocate memory");
        return 2;
    }

    tmp[0] = '\0';
    while (!done)
    {
        ch = getc(fp);
        if (ch == EOF)
        {
            c = 0;
            done = TRUE;
        }
        else if (!in_text)
        {
            if (ch == text_delim)
            {
                in_text = TRUE;
                c = 0;
            }
            else if (ch == field_delim)
            {
                done = TRUE;
            }
            else if (ch == '\n')
            {
                done = TRUE;
            }
            else
            {
                add_char(&tmp, &c, ch);
            }
        }
        else
        { /* in_text == true */
            if (esc)
            {
                if (ch == text_delim)
                {
                    add_char(&tmp, &c, ch);
                    esc = FALSE;
                }
                else
                {
                    esc = FALSE;
                    done = TRUE;
                }
            }
            else
            { /* !esc */
                if (ch == text_delim)
                {
                    esc = TRUE;
                }
                else if (ch == field_delim)
                {
                    add_char(&tmp, &c, ch);
                }
                else
                {
                    add_char(&tmp, &c, ch);
                }
            }
        }
    }

    if (field != NULL)
    {
        set_field(field, tmp);
    }

    FREE_MEMORY(tmp);
    /* Moving the fp to the beginning of the next field and peeking to see if it is a new line or if there is in fact no next field. */
    fpos_t pos;
    INT32 retval;
    done = FALSE;
    while (!done)
    {
        if (ch == field_delim)
        {
            retval = 0;
            done = TRUE;
        }
        else if (ch == '\n')
        {
            /* Checking to see if this \n is the one that UNIX sometimes includes before the EOF */
            errno = 0;
            fgetpos(fp, &pos);
            if(0 != errno)
            {
                EPRINT(UTILS, "fail to get file position");
                return 2;
            }

            ch = getc(fp);
            if (ch == EOF)
            {
                retval = 2;
            }
            else
            {
                retval = 1;
            }

            errno = 0;
            fsetpos(fp, &pos);
            if(0 != errno)
            {
                EPRINT(UTILS, "fail to set file position");
                return 2;
            }
            done = TRUE;
        }
        else if (ch == EOF)
        {
            retval = 2;
            done = TRUE;
        }
        else
        {
            ch = getc(fp);
        }
    }

    return retval;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   append_field
 * @param   buffer
 * @param   row
 * @return  return 0 on success else return non zero on failure
 */
static INT32 append_field(CSV_BUFFER *buffer, size_t row)
{
    CSV_FIELD **temp_field;

    if (buffer->rows < row + 1)
    {
        return 1;
    }

    /* Set col equal to the index of the new field */
    INT32 col = buffer->width[row];

    // PARASOFT : Release of memory for "buffer" has been taken care in "DestroyCsvBuffer" function
    temp_field = realloc(buffer->field[row], (col + 1) * sizeof(CSV_FIELD*));
    if (temp_field == NULL)
    {
        return 2;
    }
    else
    {
        buffer->field[row] = temp_field;
        buffer->field[row][col] = create_field();
        buffer->width[row]++;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   append_row
 * @param   buffer
 * @return  return 0 on success else return non zero on failure
 */
static INT32 append_row(CSV_BUFFER *buffer)
{
    size_t      *temp_width;
    CSV_FIELD   ***temp_field;

    if (buffer == NULL)
    {
        EPRINT(UTILS, "buffer ptr null");
        return -1;
    }

    size_t row  = buffer->rows;
    temp_width = realloc(buffer->width, (buffer->rows + 1) * sizeof(size_t));
    if (temp_width != NULL)
    {
        buffer->width = temp_width;
        buffer->width[row] = 0;
    }
    else
    {
        return 1;
    }

    temp_field = realloc(buffer->field, (buffer->rows + 1) * sizeof(CSV_FIELD**));
    if (temp_field != NULL)
    {
        buffer->field = temp_field;
        buffer->field[row] = NULL;
    }
    else
    {
        free(temp_width);
        return 2;
    }

    buffer->rows++;
    append_field(buffer, row);
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   remove_last_field
 * @param   buffer
 * @param   row
 * @return  return 0 on success else return non zero on failure
 */
static INT32 remove_last_field(CSV_BUFFER *buffer, size_t row)
{
    size_t      entry = buffer->width[row] - 1;
    CSV_FIELD   **temp_row;

    /* If there are no entries in the row there is nothing to remove, but return success because this is expected */
    if (row > buffer->rows - 1)
    {
        return 0;
    }

    /* If t he row exists, but has no fields, something went wrong; every row in the scope should have at least one field. */
    if (buffer->width[row] == 0)
    {
        return 1;
    }

    /* If there is only one entry left, just clear it. */
    if (buffer->width[row] == 1)
    {
        clear_field(buffer, row, 0);
        return 0;
    }

    /* Otherwise destroy the final field and decrement the width */
    destroy_field(buffer->field[row][entry]);

    // PARASOFT : Release of memory for "buffer" has been taken care in "DestroyCsvBuffer" function
    temp_row = realloc(buffer->field[row], entry * sizeof (CSV_FIELD*));
    if (temp_row != NULL)
    {
        buffer->field[row] = temp_row;
    }
    else
    {
        return 3;
    }

    buffer->width[row]--;
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   remove_last_row
 * @param   buffer
 * @return  return 0 on success else return non zero on failure
 */
static INT32 remove_last_row(CSV_BUFFER *buffer)
{
    INT32       row = buffer->rows - 1;
    INT32       entry = buffer->width[row] - 1;
    CSV_FIELD   ***temp_field = NULL;
    size_t      *temp_width = NULL;

    while (entry >= 0)
    {
        remove_last_field(buffer, row);
        entry--;
    }

    // PARASOFT : Release of memory for "buffer" has been taken care in "DestroyCsvBuffer" function
    temp_field = realloc(buffer->field, (buffer->rows - 1) * sizeof(CSV_FIELD**));

    // PARASOFT : Release of memory for "buffer" has been taken care in "DestroyCsvBuffer" function
    temp_width = realloc(buffer->width, (buffer->rows - 1) * sizeof(size_t));
    if (temp_width == NULL || temp_field == NULL)
    {
        return 1;
    }

    buffer->field = temp_field;
    buffer->width = temp_width;
    buffer->rows--;
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   copy_field
 * @param   dest
 * @param   dest_row
 * @param   dest_entry
 * @param   source
 * @param   source_row
 * @param   source_entry
 * @return  return 0 on success else return non zero on failure
 */
static INT32 copy_field(CSV_BUFFER *dest, INT32 dest_row, INT32 dest_entry, CSV_BUFFER *source, INT32 source_row, INT32 source_entry)
{
    return set_field(dest->field[dest_row][dest_entry], source->field[source_row][source_entry]->text);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   clear_field
 * @param   buffer
 * @param   row
 * @param   entry
 * @return  return 0 on success else return non zero on failure
 */
static INT32 clear_field(CSV_BUFFER *buffer, size_t row, size_t entry)
{
    /* Field is already clear (out of range) */
    if (buffer->rows < row + 1 || buffer->width[row] < entry + 1)
    {
        return 0;
    }

    /* Destroy the field if it is last in the row (and now field 0) */
    if (entry == buffer->width[row] - 1 && entry != 0)
    {
        remove_last_field(buffer, row);
    }
    else
    {
        set_field(buffer->field[row][entry], "");
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   clear_row
 * @param   buffer
 * @param   row
 * @return  return 0 on success else return non zero on failure
 */
static INT32 clear_row(CSV_BUFFER *buffer, size_t row)
{
    CSV_FIELD **temp_row;

    /* If the requested field is the last one, simply remove it. */
    if (row == buffer->rows-1)
    {
        return remove_last_row(buffer);
    }

    /* Destroy every field but the last one */
    size_t i;
    for (i = buffer->width[row] - 1; i > 0; i--)
    {
        destroy_field(buffer->field[row][i]);
    }

    /* Clear the last field */
    set_field(buffer->field[row][0], "");

    temp_row = realloc(buffer->field[row], sizeof (CSV_FIELD*));
    /* If it didn't shrink, recreate the destroyed fields */
    if (temp_row == NULL)
    {
        for (i = 1; i < buffer->width[row]; i++)
        {
            append_field(buffer, row);
            set_field(buffer->field[row][i], "");
        }
        return 1;
    }
    else
    {
        buffer->field[row] = temp_row;
    }

    buffer->width[row] = 1;
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   copy_row
 * @param   dest
 * @param   dest_row
 * @param   source
 * @param   source_row
 * @return  return 0 on success else return non zero on failure
 */
static INT32 copy_row(CSV_BUFFER *dest, size_t dest_row, CSV_BUFFER *source, size_t source_row)
{
    if (source_row > source->rows - 1)
    {
        clear_row(dest, dest_row);
        return 0;
    }

    while (dest->rows < (dest_row + 1))
    {
        if(append_row(dest) != 0)
        {
            return 1;
        }
    }

    while (dest->width[dest_row] > source->width[source_row])
    {
        if(remove_last_field(dest, dest_row) != 0)
        {
            return 1;
        }
    }

    while (dest->width[dest_row] < source->width[source_row])
    {
        if(append_field(dest, dest_row) != 0)
        {
            return 1;
        }
    }

    size_t i;
    for(i = 0; i < dest->width[dest_row]; i++)
    {
        copy_field(dest, dest_row, i, source, source_row, i);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   append_row_in_file
 * @param   fileName
 */
static void append_row_in_file(CHARPTR fileName)
{
    size_t      index = 0;
    UINT8       flag = 0;
    CHAR        lanFileName[MAX_LANGUAGE_FILE_NAME_LEN] = "";
    CHAR        sampleFileName[MAX_LANGUAGE_FILE_NAME_LEN] = "";
    CSV_BUFFER  *sample_buffer = NULL, *translated_buffer = NULL;
    CHARPTR     sample_string = NULL, translated_string = NULL;

    do
    {
        snprintf(sampleFileName, MAX_LANGUAGE_FILE_NAME_LEN, SAMPLE_LANGUAGE_FILE);
        sample_buffer = CreateCsvBuffer();
        if (NULL == sample_buffer)
        {
            break;
        }

        LoadCsvFile(sample_buffer,sampleFileName);

        snprintf(lanFileName, MAX_LANGUAGE_FILE_NAME_LEN, LANGUAGES_DIR_PATH "/%s", fileName);
        translated_buffer = CreateCsvBuffer();
        if (NULL == translated_buffer)
        {
            break;
        }

        LoadCsvFile(translated_buffer, lanFileName);

        size_t sample_string_size = MAX_LABEL_STRING_SIZE;
        sample_string = malloc(sample_string_size + 1);
        if (NULL == sample_string)
        {
            break;
        }

        GetCsvField(sample_string, sample_string_size, sample_buffer, (sample_buffer->rows - 1), 0);

        size_t translated_string_size = MAX_LABEL_STRING_SIZE;
        translated_string = malloc(translated_string_size + 1);
        if (NULL == translated_string)
        {
            break;
        }

        GetCsvField(translated_string, translated_string_size, translated_buffer, (translated_buffer->rows - 1), 0);

        // compare last English label
        if (strcmp(translated_string, sample_string) == 0)
        {
            break;
        }

        if(sample_buffer->rows > translated_buffer->rows)
        {
            size_t row = translated_buffer->rows;
            if(sample_string != NULL)
            {
                free(sample_string);
                sample_string = NULL;
            }

            for(index = 0; index < sample_buffer->rows; index++)
            {
                sample_string = malloc(sample_string_size + 1);
                if (NULL == sample_string)
                {
                    break;
                }

                GetCsvField(sample_string, sample_string_size, sample_buffer, index, 0);
                if(flag == 1)
                {
                    copy_row(translated_buffer, row, sample_buffer, index);
                    row++;
                }

                if(strcmp(translated_string, sample_string) == 0)
                {
                    flag = 1;
                }

                FREE_MEMORY(sample_string);
            }
        }
        else if(sample_buffer->rows < translated_buffer->rows)
        {
            UINT64 row = translated_buffer->rows;

            FREE_MEMORY(translated_string);
            FREE_MEMORY(sample_string);
            for(index = sample_buffer->rows; index < row; index++)
            {
                clear_row(translated_buffer, index);
            }
        }

    } while(0);

    FREE_MEMORY(translated_string);
    FREE_MEMORY(sample_string);

    if ((NULL != sample_buffer) && (NULL != translated_buffer))
    {
        SaveCsvFile(sampleFileName, sample_buffer);
        SaveCsvFile(lanFileName, translated_buffer);
        CreateTsFile(translated_buffer,lanFileName);
    }
    else
    {
        EPRINT(UTILS, "failed to create .ts file");
    }

    DestroyCsvBuffer(translated_buffer);
    DestroyCsvBuffer(sample_buffer);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
