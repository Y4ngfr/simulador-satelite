// https://github.com/DaveGamble/cJSON
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <time.h>

#define MAX_SATELLITES 10
#define MAX_APPS 10

typedef struct {
    int time;
    double x;
    double y;
} Coordenada;

// Estrutura para representar um satélite
typedef struct {
    int id;
    int cpu_capacity;
    int memory_capacity;
    Coordenada **positions;  // O time indica a posição do vetor
    double coverage_radius;
    int allocated_apps[MAX_APPS];  // Armazena os IDs das aplicações alocadas
    int allocated_count;  // Número de aplicações alocadas
} Satellite;
// strutura mais facil de percorrer
typedef struct {
    Satellite **satellites;
    int numero_satelites;
} ListaSatelites;

// Estrutura para representar uma aplicação
typedef struct {
    int id;
    int cpu_demand;
    int memory_demand;
    Coordenada position;  // Posição da aplicação
} Application;


// Função para calcular a distância Euclidiana entre o satélite e a aplicação
double calcula_distancia(Coordenada *sat_position, Coordenada app_position) {
    return sqrt(pow(sat_position->x - app_position.x, 2) + 
                pow(sat_position->y - app_position.y, 2));
}

// Função para verificar se a aplicação está dentro da cobertura do satélite
int verifica_cobertura(Satellite* sat, Application* app, int time) {
    return calcula_distancia(sat->positions[time-1], app->position) <= sat->coverage_radius;
}

// Função para verificar se o satélite tem recursos suficientes para alocar a aplicação
int pode_alocado(Satellite* sat, Application* app) {
    return sat->cpu_capacity >= app->cpu_demand && sat->memory_capacity >= app->memory_demand;
}

// Função para alocar a aplicação no satélite
void allocate(Satellite* sat, Application* app) {
    sat->cpu_capacity -= app->cpu_demand;
    sat->memory_capacity -= app->memory_demand;
    sat->allocated_apps[sat->allocated_count++] = app->id;
}

// Função para desalocar a aplicação do satélite
void deallocate(Satellite* sat, Application* app) {
    sat->cpu_capacity += app->cpu_demand;
    sat->memory_capacity += app->memory_demand;
    sat->allocated_count--;
}

// Função de backtracking para alocar as aplicações
double backtrack(Satellite **satellites, int num_satellites, Application apps[], int num_apps, int index, int allocated, int time) {
    if (index == num_apps) {
        return allocated;  // Todas as aplicações foram alocadas
    }

    Application* app = &apps[index];
    double max_allocated = allocated;
    
    printf("\nSatellites alocados - app %d: ", index);
    for (int i = 0; i < num_satellites; i++) {
        Satellite* sat = satellites[i];

        if (verifica_cobertura(sat, app, time) && pode_alocado(sat, app)) {
            allocate(sat, app);
            printf("%d", sat->id);
            max_allocated = fmax(max_allocated, backtrack(satellites, num_satellites, apps, num_apps, index + 1, allocated + 1, time));
            deallocate(sat, app);
            break;
        }
    }
    printf("\n");
    
    return max_allocated;
}

cJSON *ler_json(char *nome_arquivo) {
    FILE *arquivo;
    cJSON *json;
    char *conteudo_arquivo;
    if((arquivo = fopen(nome_arquivo, "rb")) == NULL) {
        printf("\033[41mERRO:\033[0m Não foi possivel ler o arquivo %s", nome_arquivo);
        exit(1);
    }

    fseek(arquivo, 0, SEEK_END);
    long tam_arquivo = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    conteudo_arquivo = (char*)malloc(sizeof(char) * (tam_arquivo+1));

    fread(conteudo_arquivo, 1, tam_arquivo, arquivo);
    
    json = cJSON_Parse(conteudo_arquivo);
    if(json == NULL) {
        printf("\033[41mERRO:\033[0m Não foi possivel ler o json\n");
        goto end;
    }
    fclose(arquivo);
    return json;
end:
    fclose(arquivo);
    cJSON_Delete(json);
    exit(1);
}
        


// Função de alocação gulosa
void greedy_allocate(ListaSatelites list_satellites, Application *apps, int num_apps, int time) {
    int *satellites_alocados = (int*)malloc(sizeof(int)*num_apps);
    int aux_indice = 0;
    int *aux_indice_app = (int*)malloc(sizeof(int)*list_satellites.numero_satelites),
    *aux_indice_sat = (int*)malloc(sizeof(int)*list_satellites.numero_satelites), i_aux =0; 
    for (int i = 0; i < num_apps; i++) {
        Application* app = &apps[i];
        Satellite* melhor_satellite = NULL;
        double max_resources = -1;

        for (int j = 0; j < list_satellites.numero_satelites; j++) {
            Satellite* sat = list_satellites.satellites[j];

            if (verifica_cobertura(sat, app, time) && pode_alocado(sat, app)) {
                double quantidade_recurso = sat->cpu_capacity + sat->memory_capacity;
                
                if (quantidade_recurso > max_resources) {
                    
                    max_resources = quantidade_recurso;
                    melhor_satellite = sat;
                    aux_indice_app[i_aux] = i;
                    aux_indice_sat[i_aux] = j;
                }
            }
        }
        if (melhor_satellite != NULL) {
            allocate(melhor_satellite, app);
            printf("app %d alocado no satellite %d", app->id, melhor_satellite->id);
            aux_indice++;
        } 
        printf("\n");
    }
    for(int i = 0; i < i_aux; ++i) {
        deallocate(list_satellites.satellites[aux_indice_sat[i]], &apps[aux_indice_app[i]]);
    }
}
ListaSatelites init_satellites(ListaSatelites satellites, cJSON *itens) {
    const cJSON *item = NULL;
    int tam_satelites = cJSON_GetArraySize(itens);

    satellites.numero_satelites = 0;
    satellites.satellites = (Satellite**)malloc(sizeof(Satellite*) * tam_satelites);

    cJSON_ArrayForEach(item, itens) {
        Satellite *satellite = (Satellite*)malloc(sizeof(Satellite));
        cJSON *cpu = cJSON_GetObjectItemCaseSensitive(item, "cpu");
        cJSON *memory = cJSON_GetObjectItemCaseSensitive(item, "memory");
        cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id"); 
        cJSON *range = cJSON_GetObjectItemCaseSensitive(item, "range"); 
        cJSON *coordenadas = cJSON_GetObjectItemCaseSensitive(item, "coordinates");


        cJSON *coor = NULL;
        
        satellite->positions = (Coordenada**)malloc(sizeof(Coordenada*) * cJSON_GetArraySize(coordenadas));
        cJSON_ArrayForEach(coor, coordenadas) {
            Coordenada *coordenada = (Coordenada*)malloc(sizeof(Coordenada));
            cJSON *time = cJSON_GetObjectItemCaseSensitive(coor, "time");
            cJSON *coordinates = cJSON_GetObjectItemCaseSensitive(coor, "coordinates");
            cJSON *c = NULL;
            double x = 0.0, y = 0.0;
            int i = 0;

            cJSON_ArrayForEach(c, coordinates) {
                if (i == 0) {
                    x = c->valuedouble;
                } else {
                    y = c->valuedouble;
                }
                i++;
            }

            coordenada->time = time->valueint;
            coordenada->x = x;
            coordenada->y = y;
            satellite->positions[coordenada->time - 1] = coordenada;
        }

        satellite->cpu_capacity = cpu->valueint;
        satellite->id = id->valueint;
        satellite->memory_capacity = memory->valueint;
        satellite->coverage_radius = (range->valuedouble) / 2;  // Raio
        satellite->allocated_count = 0;

        satellites.satellites[satellites.numero_satelites] = satellite;
        satellites.numero_satelites++;
    }

    return satellites;
}


void imprime_infos_app_teste(Application *apps, int num_apps) {
    printf("========== INFORMAÇÔES SOBRE OS APPs ==========\n");
    for(int i = 0; i < num_apps; ++i) {
        printf("--------------------------------------------\n");
        printf("id: %d\n", apps[i].id);
        printf("cpu_demand: %d\n", apps[i].cpu_demand);
        printf("memory_demand: %d\n", apps[i].memory_demand);
        printf("posicao - x: %f y: %f\n", apps[i].position.x, apps[i].position.y); 
    }
    printf("--------------------------------------------\n");
}   


int main() {
    cJSON *json;
    FILE *arquivo = NULL;
    ListaSatelites satellites;
    time_t t_inicio, t_final;
    cJSON *itens;
    double t_back, t_greed;
    int num_satellites = 2, num_apps = 2, aux = 1, tempo = 1;

    srand(time(NULL));

    // Application apps[num_apps];
    // for(int i = 0; i < num_apps; ++i) {
    //     apps[i].id = i;
    //     apps[i].cpu_demand = (rand()%100)+1;
    //     apps[i].memory_demand = (rand()%53)+1;
    //     apps[i].position.time = 0;
    //     apps[i].position.x =  37.769655522217555;
    //     apps[i].position.y = -122.4211555521247;
    // }

    Application apps[2] = {
        {1, 80, 50, {0, 37.769655522217555, -122.4211555521247}}, 
        {2, 80, 50, {0, 37.769655522217555, -122.4211555521247}}
    };

    imprime_infos_app_teste(apps, num_apps);

    while(tempo <= 20 && aux <= 2) {
        char arquivo_nome[100];
        printf("--------------------------------------------\n");
        printf("rodando teste no arquivo log%d.json e tempo %d\n", aux, tempo);
        if(tempo == 1) {
            sprintf(arquivo_nome, "./inputs/log%d.json", aux);
            json = ler_json(arquivo_nome);
            itens = cJSON_GetObjectItemCaseSensitive(json, "satellites");

            if (itens == NULL) {
                goto end;
            }

            satellites = init_satellites(satellites, itens);
        }

        t_inicio = time(NULL);
        printf("\n====== iniciando (backtrack) ======\n");
        printf("maximo de alocações: %f\n\n", backtrack(satellites.satellites, satellites.numero_satelites, apps, num_apps, 0, 0, tempo));
        time(&t_final);
        t_final = time(NULL);
        t_back = difftime(t_final, t_inicio);

        t_inicio = time(NULL);
        satellites = init_satellites(satellites, itens);
        printf("\n====== iniciando (greedy_allocate) ======\n\n");
        greedy_allocate(satellites, apps, num_apps, tempo);
        t_final = time(NULL);

        t_greed =  difftime(t_final, t_inicio);

        printf("\ntempo execucao (backtrack): %.f\n", t_back);

        printf("tempo execucao (greedy_allocate): %.f\n\n", t_greed);
        tempo++;
        if((tempo % 21) == 0) {
            aux++;
            tempo = 1; 
        }
    }
    printf("--------------------------------------------\n");
    return 0;
end:
    cJSON_Delete(itens);
    cJSON_Delete(json);
    exit(1);
}