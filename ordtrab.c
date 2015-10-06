#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "import.h"

//Registro
typedef struct {
    int id_1;
    char str_1[50];
    int id_2;
    char str_2[50];
}reg;

//indice secundário
typedef struct {
    int id;
    int rrn;
}idx;

//Indice primário
typedef struct {
    int pid;
    int ref;
}pidx;

//Lista invertida
typedef struct {
    int id;
    int prox;
}inv;

//indice primário com lista invertida
typedef struct {
    int pid;
    int ref;
    int rrn;
}pidxinv;

int pidxcmp(const void *v1, const void *v2) {
    const pidx *p1 = (pidx *)v1;
    const pidx *p2 = (pidx *)v2;

    if (p1->pid < p2->pid) {
        return -1;
    } else if (p1->pid > p2->pid) {
        return +1;
    } else {
        return 0;
    }
}

int idxcmp(const void *v1, const void *v2) {
    const idx *p1 = (idx *)v1;
    const idx *p2 = (idx *)v2;

    if (p1->id < p2->id) {
        return -1;
    } else if (p1->id > p2->id) {
        return +1;
    } else {
        return 0;
    }
}

int pidxinvcmp(const void *v1, const void *v2) {
    const pidxinv *p1 = (pidxinv *)v1;
    const pidxinv *p2 = (pidxinv *)v2;

    if (p1->pid < p2->pid) {
        return -1;
    } else if (p1->pid > p2->pid) {
        return +1;
    } else {
        return 0;
    }
}

void write_primary_file (pidx primary[], int num_reg) {
    FILE *output;
    int i;
    
    if((output = fopen("primary.txt","w")) == NULL) {
        printf("Não foi possível abrir o arquivo para gravar o Índice Primário.");
        exit(1);
    }

    for (i = 0; i < num_reg; i++) {
        fprintf (output, "%d\n%d\n", primary[i].pid, primary[i].ref);
    }

    fclose(output);
}

void write_secondary_file (idx secondary[], int s_size) {
    FILE *output;
    int i;
    
    if((output = fopen("secondary.txt","w")) == NULL) {
        printf("Não foi possível abrir o arquivo para gravar o Índice Secundário.");
        exit(1);
    }

    for (i = 0; i < s_size; i++) {
        fprintf (output, "%d\n%d\n", secondary[i].id, secondary[i].rrn);
    }

    fclose(output);
}

void write_primary_inverse (pidxinv primary[], int num_reg) {
    FILE *output;
    int i;

    if((output = fopen("primaryinv.txt", "w")) == NULL) {
        printf("Não foi possível abrir o arquivo para gravar o Índice Primário invertido.");
        exit(1);
    }

    for (i = 0; i < num_reg; i++) {
        fprintf (output, "%d\n%d\n%d\n", primary[i].pid, primary[i].ref, primary[i].rrn);
    }
    
    fclose(output);
}

short get_str(FILE *input, char str[]) {
        short rec;

	if (feof(input)) {
		return 0;
	}

	fread(&rec, sizeof(rec), 1, input);
	fread(str, rec, 1, input);

	return rec;
}

int get_fld(char field[], char buffer[], short* scan_pos, short* str_lenght) {
	short f_pos = 0;
	char ch = '\0';

        memset(field,0,sizeof(field));

	if (*scan_pos == *str_lenght) {
		return 0;
	}
	
	//Get a ch at the SCAN_POS in the BUFFER
	ch = buffer[*scan_pos];

	while (((*scan_pos) < (*str_lenght)) && ('|' != ch)) {
	    field[f_pos] = ch;
	    f_pos++;
	    (*scan_pos)++;
	    ch = buffer[*scan_pos];
	}

        (*scan_pos)++;

	field[strlen(field)+1] = '\0';

	return *scan_pos;
}

void create_primary_idx(pidx primary[], char filename[], int num_reg) {
    FILE *input;
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght, aux;
    int max_lenght = 0;
    int i;

    if((input = fopen(filename, "r")) == NULL) {
        printf("Arquivo Incorreto!");
        exit(1);
    }

    str_lenght = get_str(input, buffer);
    aux = str_lenght;

    for (i = 0; i < num_reg; i++) {
        scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);

        primary[i].pid = atoi(field);
        primary[i].ref = max_lenght;

        max_lenght += aux + 2;
        str_lenght = get_str(input, buffer);
        aux = str_lenght;
        scan_pos = 0;
    }

    fclose(input);
}

int exist_id_pidx(int is, pidxinv primary[], int num_reg) {
    int i;

    for(i = 0; i < num_reg; i++) {
        if (primary[i].pid == is) {
            return i;
        }
    }
}

void create_primary_inversed_idx(pidxinv primary[], char filename[], int num_reg1, int num_reg2, reg registers[], inv inverted[]) {
        FILE *input;
        char buffer[512], field[50];
        short scan_pos = 0;
        short str_lenght, aux;
        short max_lenght = 0;
        int i;

        int pos;
        int p_pos = 0;
        int rrn;

        if((input = fopen(filename, "r")) == NULL) {
            printf("Arquivo Incorreto!");
            exit(1);
        }

        str_lenght = get_str(input, buffer);
        aux = str_lenght;

        for (i = 0; i < num_reg1; i++) {
            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);

            primary[i].pid = atoi(field);
            primary[i].ref = max_lenght;
            primary[i].rrn = -1;

            //printf("%d %d\n", primary[i].pid, primary[i].ref);
            
            max_lenght += aux + 2;
            str_lenght = get_str(input, buffer);
            aux = str_lenght;
            scan_pos = 0;
        }

        fclose(input);

        for(i = 0; i < num_reg2; i++) {
            pos = exist_id_pidx(registers[i].id_2, primary, num_reg1);
            
            if((rrn = primary[pos].rrn) == -1) {
                primary[pos].rrn = i;
                inverted[i].id = registers[i].id_1;
                inverted[i].prox = -1;
            } else {
                inverted[i].id = registers[i].id_1;
                inverted[i].prox = -1;

                while (inverted[rrn].prox != -1) {
                   rrn = inverted[rrn].prox; 
                }
                inverted[rrn].prox = i;
            }
        }
    }


    /* Verifica se o id existe no indice secundário */
    int exist_id_idx(int is, idx secondary[], int s_pos) {
        int i;
        
        //Implementar busca binária
        for (i = 0; i < s_pos; i++) {
            if (secondary[i].id == is) {
                return secondary[i].rrn;
            }
        }
        return -2;
    }

    /* Cria um índice secundário e retorn a lista invertida */
    int create_secondary_idx(int num_reg, reg registers[], idx secondary[], inv inverted[]) {
        int i, j, rrn;
        int s_pos = 0;
        
        for(i = 0; i < num_reg; i++) {
            if ((rrn = exist_id_idx(registers[i].id_2, secondary, s_pos)) == -2) {
                secondary[s_pos].id = registers[i].id_2;
                secondary[s_pos].rrn = i;

                //printf("Entroooou %d\n", registers[i].id_2);

                inverted[i].id = registers[i].id_1;
                inverted[i].prox = -1;

                s_pos++;
            } else {
                //Necessário ordenar lista encadeada ainda
                inverted[i].id = registers[i].id_1;
                inverted[i].prox = -1; 
                while (inverted[rrn].prox != -1) {
                    //printf("%d -> ", inverted[rrn].id);
                    rrn = inverted[rrn].prox;
                }
                inverted[rrn].prox = i;
            }
        //printf("%d\n",inverted[i].id);
        }

        return s_pos;
    }

    int insert_reg_i (int idi, int idr, char nome[], char sexo[], char filename[]) {
        FILE* output;
        char buffer[512];
        char buf[50];
        short reg_lenght;

        if((output = fopen(filename,"a+")) == NULL) {
            printf("ERRO: Arquivo incorreto!");
            exit(1);
        }

        buffer[0] = '\0';
        buf[0] = '\0';
        
        sprintf(buf, "%d", idi);
        strcat(buffer, buf);
        strcat(buffer,"|");
        buf[0] = '\0';

        sprintf(buf, "%d", idr);
        strcat(buffer,buf);
        strcat(buffer,"|");
        buf[0] = '\0';

        strcat(buffer, nome);
        strcat(buffer, "|");

        strcat(buffer, sexo);
        strcat(buffer, "|");

        reg_lenght = strlen(buffer);

        printf("%s", buffer);

        fwrite(&reg_lenght, 1, sizeof(reg_lenght), output);
        fwrite(buffer, 1, reg_lenght, output);

        fclose(output);
    }

int scan_reg_i(reg registers[], char filename[], FILE *input) {
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;

    int num_reg = 0;

    if((input = fopen(filename,"r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    str_lenght = get_str(input, buffer);

    while (str_lenght > 0) {
        scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght); 

        while (scan_pos > 0) {
            registers[num_reg].id_1 = atoi(field);

	    scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            registers[num_reg].id_2 = atoi(field);
                
            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(registers[num_reg].str_1, field);

	    scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(registers[num_reg].str_2, field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
        }
	str_lenght = get_str(input, buffer);
        num_reg++;
    }
	
    fclose(input);
    return num_reg;
}

int scan_reg_r(reg registers[], char filename[], FILE *input) {
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;

    int num_reg = 0;

    if((input = fopen(filename,"r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    str_lenght = get_str(input, buffer);

    while (str_lenght > 0) {
        scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght); 

        while (scan_pos > 0) {
            registers[num_reg].id_1 = atoi(field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(registers[num_reg].str_1, field);

	    scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            registers[num_reg].id_2 = atoi(field); 

	    scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(registers[num_reg].str_2, field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
        }
	str_lenght = get_str(input, buffer);
        num_reg++;
    }
	
    fclose(input);
    return num_reg;
}

/* Buscas */

//usa ip1 
 busca_ip_idr(int chave, pidxinv index[], int idx_size, char filename[]) {
    FILE *input;
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;
    int i;
    
    if((input = fopen(filename,"r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    for(i = 0; i < idx_size; i++) {
        if (index[i].pid == chave) {
            fseek(input, index[i].ref, SEEK_SET);

            str_lenght = get_str(input, buffer);

            //Raças

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Identificador da Raça: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Nome da Raça: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Identificador de Grupo: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Nome do grupo: %s\n\n", field);

        }
    }

    fclose(input);
}

//is
void busca_is(int chave, idx index[], inv inverted[], int s_size ,int num_reg_inv, pidxinv pid_index[], char filename[]) {
    int i;
    int rrn;
    int rrn_key;

    for(i = 0; i < s_size; i++) {
        if(index[i].id == chave) {
            rrn = index[i].rrn;
            while(rrn != -1) {
                rrn_key = inverted[rrn].id;

                busca_ip_idr(rrn_key, pid_index, num_reg_inv, filename);

                rrn = inverted[rrn].prox;
            }
        }
    }
}

void busca_ip_in(int chave, pidx p_idx[], int num_reg_i, char filename[]) {
    FILE *input;
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;
    int i;

    if((input = fopen(filename,"r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    for (i = 0; i < num_reg_i; i++) {
        if(p_idx[i].pid == chave) {
            fseek(input, p_idx[i].ref, SEEK_SET);

            str_lenght = get_str(input, buffer);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Identificador do indivíduo: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Identificador da raça: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Nome do indivíduo: %s\n", field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            printf("Sexo: %s\n\n", field);
        }
    }
}

//Busca de indivíduos pelo id-r
void busca_ls_ir(int chave, pidxinv index[], inv inverted[], int idx_size, char filename[], pidx p_idx[], int num_reg_i) {
    int i, rrn;
    int rrn_key;

    for(i = 0; i < idx_size; i++) { 
        if(index[i].pid == chave) {
            rrn = index[i].rrn;
            while(rrn != -1) {
                rrn_key = inverted[rrn].id;

                busca_ip_in(rrn_key, p_idx, num_reg_i, filename);

                rrn = inverted[rrn].prox;
            }
        }
    }
}

void busca_sexo(int chave, char sec_key[], pidx p_idx[], int num_reg_i, char filename[]) {
    FILE *input;
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;
    int i;

    int id_1, id_2;
    char str_1[50], str_2[50];

    if((input = fopen(filename,"r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    for (i = 0; i < num_reg_i; i++) {
        if(p_idx[i].pid == chave) {
            fseek(input, p_idx[i].ref, SEEK_SET);

            str_lenght = get_str(input, buffer);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            id_1 = atoi(field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            id_2 = atoi(field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(str_1, field);

            scan_pos = get_fld(field, buffer, &scan_pos, &str_lenght);
            strcpy(str_2, field);

            if (strcmp(str_2, sec_key) == 0) {
                printf("Identificador do indivíduo: %d\n", id_1);
                printf("Identificador da Raça: %d\n", id_2);
                printf("Nome do individuo: %s\n", str_1);
                printf("Sexo: %s\n\n", str_2);
            }
        }
    }
}

//Busca de indivíduos pelo id-r e sexo
void busca_ls(int chave, char sec_key[], pidxinv index[], inv inverted[], int idx_size, char filename[], pidx p_idx[], int num_reg_i) {
    FILE *input;
    char buffer[512], field[50];
    short scan_pos = 0;
    short str_lenght;

    int i, rrn;
    int rrn_key;

    if((input = fopen(filename, "r")) == NULL) {
        printf("Arquivo incorreto.");
        exit(1);
    }

    for(i = 0; i < idx_size; i++) { 
        if(index[i].pid == chave) {
            rrn = index[i].rrn;
            while(rrn != -1) {
                rrn_key = inverted[rrn].id;

                busca_sexo(rrn_key, sec_key, p_idx, num_reg_i, filename);

                rrn = inverted[rrn].prox;
            }
        }
    }
}

int main() {
        FILE *input_i, *input_r;
	char individuos[20], raca[20];
        
        int num_reg_i, num_reg_r;
        int s_size;
        int op,in;

        int id_1, id_2;

        char str_1[50], str_2[50], aux;
        
        reg *registers_i = malloc (100 * sizeof(reg));
        reg *registers_r = malloc (100 * sizeof(reg));
       
        idx *is2 = malloc (100 * sizeof(idx));


        if ((input_i = fopen("primary.txt", "r+")) != NULL) {
            printf("Arquivo detectado! Reconstruindo índices!");    
            
            strcpy(individuos, "individuos.txt");
            strcpy(raca, "racas-grupos.txt");

            num_reg_i = scan_reg_i (registers_i, individuos, input_i);
            num_reg_r = scan_reg_r (registers_r, raca, input_r);
        } else {
            printf("Digite o nome do arquivo de indivíduos: ");
            fgets(individuos, sizeof(individuos), stdin);
            strtok(individuos,"\n");

            printf("\nDigite o nome do arquivo de raças: ");
            fgets(raca, sizeof(raca), stdin);
            strtok(raca, "\n");

            importFile(individuos);
            importFile(raca);

            num_reg_i = scan_reg_i (registers_i, individuos, input_i);
            num_reg_r = scan_reg_r (registers_r, raca, input_r); 
        }

        registers_i = realloc (registers_i, num_reg_i * sizeof(reg));
        registers_r = realloc (registers_r, num_reg_r * sizeof(reg));


         /* Criando índice primário ip3 */
        pidx ip3[num_reg_i]; 

        create_primary_idx(ip3, individuos, num_reg_i);
        qsort(ip3, num_reg_i, sizeof(pidx), pidxcmp);

        write_primary_file(ip3, num_reg_i);

        /* Criando indice secundário e lista invertida is2 */
        inv is2_inverted[num_reg_r];

        s_size = create_secondary_idx(num_reg_r, registers_r, is2, is2_inverted);
        is2 = realloc (is2, s_size * sizeof(idx));
        qsort(is2, s_size, sizeof(idx), idxcmp);

        write_secondary_file(is2, s_size);

        /* Criando indice primário e lista invertida ip1 */
        inv ip1_inverted[num_reg_i];
        pidxinv ip1[num_reg_r];
    
        create_primary_inversed_idx(ip1, raca, num_reg_r, num_reg_i, registers_i, ip1_inverted);
        qsort(ip1, num_reg_r, sizeof(pidxinv), pidxinvcmp);

        write_primary_inverse(ip1, num_reg_r);

        while (1) {

        printf("Programa para cadastro de Cães. O que deseja fazer? \n\n");
        printf("1 Importar\n2 Inserir indivíduo\n3 Buscar\n4 Sair\n\n Insira sua escolha: ");
        scanf("%d", &op);

        if(op == 1) {
            //free(is2);
            //free(registers_r);
            //free(registers_i);

            remove(individuos);
            remove(raca);

            is2 = malloc(100 * sizeof(idx));
            registers_r = malloc (100 * sizeof(reg));
            registers_i = malloc (100 * sizeof(reg));

	    printf("Digite o nome do arquivo de indivíduos: ");
            fgets(individuos, sizeof(individuos), stdin);
            strtok(individuos,"\n");

            printf("\n\nDigite o nome do arquivo de raças: ");
            fgets(raca, sizeof(raca), stdin);
            strtok(raca, "\n");

            importFile(individuos);
            importFile(raca);
        
            num_reg_i = scan_reg_i (registers_i, individuos, input_i);
            num_reg_r = scan_reg_r (registers_r, raca, input_r);

            registers_i = realloc (registers_i, num_reg_i * sizeof(reg));
            registers_r = realloc (registers_r, num_reg_r * sizeof(reg));

            //criando indices
            create_primary_idx(ip3, individuos, num_reg_i);
            qsort(ip3, num_reg_i, sizeof(pidx), pidxcmp);

            write_primary_file(ip3, num_reg_i);

            //
            s_size = create_secondary_idx(num_reg_r, registers_r, is2, is2_inverted);
            is2 = realloc (is2, s_size * sizeof(idx));
            qsort(is2, s_size, sizeof(idx), idxcmp);

            write_secondary_file(is2, s_size);

            //
            create_primary_inversed_idx(ip1, raca, num_reg_r, num_reg_i, registers_i, ip1_inverted);
            qsort(ip1, num_reg_r, sizeof(pidxinv), pidxinvcmp);

            write_primary_inverse(ip1, num_reg_r); 


        } else if (op == 2) {

            printf("Digite os dados do indivíduo a ser adicionado: \n\n");
            printf("Identificador do indivíduo: ");
            scanf("%d", &id_1);

            printf("\nIdentificador de Raça: ");
            scanf("%d", &id_2);

            printf("\nNome: ");
            //while ((aux = getchar()) != EOF && aux != '\n');
            __fpurge(stdin);
            fgets(str_1, sizeof(str_1), stdin);
            str_1[strlen(str_1) -1] = '\0';

            printf("\nSexo: ");
            //while ((aux = getchar()) != EOF && aux != '\n');
            __fpurge(stdin);
            fgets(str_2, sizeof(str_2), stdin);
            str_2[strlen(str_2) - 1] = '\0';
            
            int i;
            for(i = 0; i < num_reg_i; i++) {
                if (ip1[i].pid == id_1) {
                    printf("\nERRO! Este id já existe\n\n");
                    i = -1;
                    break;
                }
            }

            if (i != -1) {
                insert_reg_i(id_1, id_2, str_1, str_2, individuos);
                printf("Adicionado com sucesso!\n");
 
                //free(is2);

                registers_r = realloc (registers_r, (num_reg_i + 1) * sizeof(reg));
                is2 = malloc(100 * sizeof(idx));

                num_reg_i = scan_reg_i (registers_i, individuos, input_i);

                //criando indices
                create_primary_idx(ip3, individuos, num_reg_i);
                qsort(ip3, num_reg_i, sizeof(pidx), pidxcmp);

                //
                s_size = create_secondary_idx(num_reg_r, registers_r, is2, is2_inverted);
                is2 = realloc (is2, s_size * sizeof(idx));
                qsort(is2, s_size, sizeof(idx), idxcmp);

                //
                create_primary_inversed_idx(ip1, raca, num_reg_r, num_reg_i, registers_i, ip1_inverted);
                qsort(ip1, num_reg_r, sizeof(pidxinv), pidxinvcmp);
 
            }

        } else if (op == 3) {
            printf("\nLista de buscas: \n\n");
            printf("    1 - Registro de raça pelo ID-R\n");
            printf("    2 - Lista de raças pelo ID-G\n");
            printf("    3 - Registro de indivíduo pelo ID-I\n");
            printf("    4 - Lista de indivíduos pelo ID-R\n");
            printf("    5 - Lista de indivíduos pelo ID-R e Sexo\n\n");
            printf("Digite sua opção: ");
            scanf("%d", &op);

            if (op == 1) {
                printf("Digite o ID-R: ");
                scanf("%d", &in);
                
                busca_ip_idr(in, ip1, num_reg_r, raca);
            } else if (op == 2) {
                printf("Digite o ID-G: ");
                scanf("%d", &in);

                busca_is(in, is2, is2_inverted, s_size, num_reg_r, ip1, raca);
            } else if (op == 3) {
                printf("Digite o ID-I: ");
                scanf("%d", &in); 

                busca_ip_in(in, ip3, num_reg_i, individuos);
            } else if (op == 4) {
                printf("Digite o ID-R: ");
                scanf("%d", &in); 

                busca_ls_ir(in, ip1, ip1_inverted, num_reg_r, individuos, ip3, num_reg_i);
            } else if (op == 5) {
                printf("Digite o ID-R: ");
                scanf("%d", &in); 
                while ((aux = getchar()) != EOF && aux != '\n');
                printf("Digite o Sexo: ");
                fgets(str_1, sizeof(str_1), stdin);

                busca_ls(in, str_1, ip1, ip1_inverted, num_reg_r, individuos, ip3, num_reg_i);
            } else {
                printf("Número incorreto.");
            }
        } else if (op == 4) {
        
            write_primary_file(ip3, num_reg_i);
            write_secondary_file(is2, s_size);
            write_primary_inverse(ip1, num_reg_r);

            break;
        }
        }

        FILE *saida;

        saida = fopen("lista.txt", "w");

        int i;
        for (i = 0; i < num_reg_r; i++) {
            fprintf(saida,"%d %d\n",is2_inverted[i].id, is2_inverted[i].prox);
        }

        fclose(saida);

        saida = fopen("lista_i.txt", "w");

        for (i = 0; i < num_reg_i; i++) {
            fprintf(saida, "%d %d\n", ip1_inverted[i].id, ip1_inverted[i].prox);
        }

        fclose(saida);

        printf("\n\nAdieu! :)\n");

        free(registers_i);
        free(registers_r);
        free(is2);

       	return 0;
}
