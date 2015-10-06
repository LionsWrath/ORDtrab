#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void importFile(char filename[]) {
    FILE* input;
    FILE* output;
    char buffer[512];
    char *line = NULL;
    short reg_lenght;
    size_t len = 0;
    ssize_t read;

    /* Verificar se o arquivo de saída é válido */
    input = fopen(filename,"r");
    if (input == NULL) {
        printf("ERRO: Arquivo incorreto.");
        exit(1);
    }

    output = fopen("output.txt","w");
    if (output == NULL) {
        printf("ERRO: Arquivo incorreto.");
        exit(1);
    }

    /* Começar a ler arquivo linha por linha. */
    read = getline(&line, &len, input);
    while (read != -1) {
        /* Reiniciar o Buffer */
        buffer[0] = '\0';

        strtok(line,"\n");
        strtok(line,"\r");
        strcat(buffer, line);
        strcat(buffer, "|");

        getline(&line, &len, input);
        strtok(line,"\n");
        strtok(line,"\r");
        strcat(buffer, line);
        strcat(buffer, "|");

        getline(&line, &len, input);
        strtok(line,"\n");
        strtok(line,"\r");
        strcat(buffer, line);
        strcat(buffer, "|");
       
        getline(&line, &len, input);
        strtok(line,"\n");
        strtok(line,"\r");
        strcat(buffer, line);
        strcat(buffer, "|");

        /* Após pegar os dados do registro, guardá-lo no formato desejado */
        reg_lenght = strlen(buffer);

        fwrite(&reg_lenght, 1, sizeof(reg_lenght), output);
        fwrite(buffer, 1, reg_lenght, output);

        read = getline(&line, &len, input);
    }

    /* Limpando os restos */
    fclose(input);
    remove(filename);

    fclose(output);
    rename("output.txt",filename);

    free(line);
}
