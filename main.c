// https://github.com/DaveGamble/cJSON
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cjson/cJSON.h>

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
    double position[2];  // Posição da aplicação
} Application;

typedef struct {
    Application **applications;
    int numero_application;
} ListaApplication;

// Função para calcular a distância Euclidiana entre o satélite e a aplicação
double calculate_distance(Coordenada * sat_position, double app_position[3]) {
    return sqrt(pow(sat_position->x - app_position[0], 2) + 
                pow(sat_position->y - app_position[1], 2));
}

// Função para verificar se a aplicação está dentro da cobertura do satélite
int is_within_coverage(Satellite* sat, Application* app, int time) {
    return calculate_distance(sat->positions[time-1], app->position) <= sat->coverage_radius;
}

// Função para verificar se o satélite tem recursos suficientes para alocar a aplicação
int can_allocate(Satellite* sat, Application* app) {
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

    for (int i = 0; i < num_satellites; i++) {
        Satellite* sat = satellites[i];

        if (is_within_coverage(sat, app, time) && can_allocate(sat, app)) {
            allocate(sat, app);
            max_allocated = fmax(max_allocated, backtrack(satellites, num_satellites, apps, num_apps, index + 1, allocated + 1, time));
            deallocate(sat, app);
        }
    }

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
void greedy_allocate(Satellite **satellites, int num_satellites, Application *apps, int num_apps, int time) {
    for (int i = 0; i < num_apps; i++) {
        Application* app = &apps[i];
        Satellite* best_satellite = NULL;
        double max_resources = -1;

        // Procurar o satélite mais adequado para a aplicação
        for (int j = 0; j < num_satellites; j++) {
            Satellite* sat = satellites[j];

            // Verificar se o satélite está dentro do alcance e tem recursos suficientes
            if (is_within_coverage(sat, app, time) && can_allocate(sat, app)) {
                double available_resources = sat->cpu_capacity + sat->memory_capacity;
                
                // Verificar se esse satélite tem mais recursos livres
                if (available_resources > max_resources) {
                    max_resources = available_resources;
                    best_satellite = sat;
                }
            }
        }

        // Se encontramos um satélite adequado, alocar a aplicação nele
        if (best_satellite != NULL) {
            allocate(best_satellite, app);
            printf("Aplicação %d alocada no satélite %d\n", app->id, best_satellite->id);
        } else {
            printf("Aplicação %d não foi alocada\n", app->id);
        }
    }
}

int main() {
    cJSON *json;
    FILE *arquivo = NULL;
    const cJSON *item = NULL;
    ListaSatelites satellites;

    Application apps[MAX_APPS] = {
        {1, 30, 30, {37.769655522217555, -122.4211555521247}},
        {2, 80, 150, {20, 20}}
    };

    int num_satellites = 2;
    int num_apps = 2;

    // Lê o arquivo JSON
    json = ler_json("./inputs/log1.json");
    cJSON *itens = cJSON_GetObjectItemCaseSensitive(json, "satellites");

    int tam_satelites = cJSON_GetArraySize(itens);

    satellites.numero_satelites = 0;
    satellites.satellites = (Satellite**)malloc(sizeof(Satellite*) * tam_satelites);

    if (itens == NULL) {
        goto end;
    }

    cJSON_ArrayForEach(item, itens) {
        Satellite *satellite = (Satellite*)malloc(sizeof(Satellite));
        cJSON *cpu = cJSON_GetObjectItemCaseSensitive(item, "cpu");
        cJSON *memory = cJSON_GetObjectItemCaseSensitive(item, "memory");
        cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id"); 
        cJSON *range = cJSON_GetObjectItemCaseSensitive(item, "range"); 
        cJSON *coordenadas = cJSON_GetObjectItemCaseSensitive(item, "coordinates");

        int time_couter = 1;
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
            }

            coordenada->time = time->valueint;

            satellite->positions[time_couter - 1] = coordenada;
        }

        satellite->cpu_capacity = cpu->valueint;
        satellite->id = id->valueint;
        satellite->memory_capacity = memory->valueint;
        satellite->coverage_radius = (range->valuedouble) / 2;  // Raio
        satellite->allocated_count = 0;

        satellites.satellites[satellites.numero_satelites] = satellite;
        satellites.numero_satelites++;
    }

    // Alocação gulosa das aplicações
    greedy_allocate(satellites.satellites, satellites.numero_satelites, apps, num_apps, 1);

    return 0;
end:
    cJSON_Delete(itens);
    cJSON_Delete(json);
    exit(1);
}