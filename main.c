#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"


int main() {
    char comando[256];
    Dataframe *dfActivo = NULL; // Dataframe activo
    Lista listaDeDataframes = {0, NULL}; // Lista global de dataframes
    printf("Alejandro Gallego Lopez, alejandro.gallego@goumh.umh.es\n"); //Datos del alumno

    while (1) {

        // Mostrar el prompt
        if (dfActivo) {
            TextColor(WHITE);
            printf("[%s: %d,%d]:> ", dfActivo->nombre, dfActivo->numFilas, dfActivo->numColumnas);
        } else {
            TextColor(WHITE);
            printf("[?]:> ");
        }

        // Leer el comando ingresado por el usuario
        if (!fgets(comando, sizeof(comando), stdin)) {
            continue; // Si hay un error al leer, reiniciar el loop
        }

        // Eliminar saltos de linea del comando
        comando[strcspn(comando, "\n")] = 0;

        // Comprobar si el comando es nulo o vacio
        if (strlen(comando) == 0) {
            continue; // No hacer nada, volver al prompt
        }

        // Manejar comandos
        if (strncmp(comando, "quit", 4) == 0) {
            // Comando "quit": liberar memoria y volver al estado inicial
            TextColor(GREEN);
            printf("EXIT PROGRAM\n");

            // Liberar toda la memoria din�mica asociada a los dataframes
            Nodo *nodo = listaDeDataframes.primero;
            while (nodo) {
                Nodo *temp = nodo;
                nodo = nodo->siguiente;
                freeDataframe(temp->df);
                free(temp);
            }
            listaDeDataframes.primero = NULL;
            listaDeDataframes.numDFs = 0;

            // Restablecer el dataframe activo a NULL
            dfActivo = NULL;

            // Restablecer el color del texto al predeterminado
            TextColor(WHITE);

            // Anadir una linea vacia para limpiar la salida
            printf("\n");

            // Continuar el programa mostrando el prompt inicial
            continue;

        } else if (strncmp(comando, "Load", 4) == 0) {
            // Comando "load": cargar un archivo CSV como dataframe
            char filename[256];
            if (sscanf(comando + 5, "%s", filename) != 1) {
                TextColor(RED);
                printf("Error: Debe especificar un archivo para cargar.\n");
                continue;
            }

            Dataframe *nuevoDF = Load(filename, ',', 1);
            if (!nuevoDF) {
                TextColor(RED);
                printf("Error al cargar el dataframe.\n");
            } else {
                // Asignar un nombre por defecto
                char nombre[51];
                snprintf(nombre, sizeof(nombre), "df%d", listaDeDataframes.numDFs);
                strcpy(nuevoDF->nombre, nombre);

                // Anadir el dataframe a la lista
                Nodo *nuevoNodo = malloc(sizeof(Nodo));
                nuevoNodo->df = nuevoDF;
                nuevoNodo->siguiente = listaDeDataframes.primero;
                listaDeDataframes.primero = nuevoNodo;
                listaDeDataframes.numDFs++;

                // Cambiar el dataframe activo
                dfActivo = nuevoDF;

                TextColor(GREEN);
                printf("Dataframe cargado como '%s'.\n", dfActivo->nombre);
                TextColor(WHITE);
            }

        } else if (strncmp(comando, "view", 4) == 0) {
            int n = 10; // Valor predeterminado de 'n'
            char *param = comando + 5;

            // Verificar si hay un parametro despues de 'view'
            if (strlen(param) > 0 && sscanf(param, "%d", &n) != 1) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: view [n]\n");
                TextColor(WHITE);
                continue;
            }

            // Verificar si hay un dataframe activo
            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            // Llamar a la función viewDataframe con el valor de 'n'
            viewDataframe(dfActivo, n);

        } else if (strncmp(comando, "meta", 4) == 0) {
            // Comando "meta": mostrar metadatos del dataframe activo
            if (strlen(comando) > 4) {
                TextColor(RED);
                printf("Error: El comando 'meta' no lleva parametros.\n");
                TextColor(WHITE);
                continue;
            }

            // Verificar si hay un dataframe activo
            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            // Mostrar los metadatos del dataframe activo
            metaDataframe(dfActivo);

        } else if (strncmp(comando, "save", 4) == 0) {
            // Comando "save": guardar el dataframe activo
            char filename[256];
            if (sscanf(comando + 5, "%s", filename) != 1) {
                TextColor(RED);
                printf("Error: Debe especificar un nombre de archivo.\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            saveDataframe(dfActivo, filename);
            TextColor(GREEN);
            printf("Dataframe guardado en '%s'.\n", filename);
            TextColor(WHITE);

        } else if (strncmp(comando, "filter", 6) == 0) {
            // Comando "filter": filtrar filas por columna
            char colName[30], op[4], value[256];
            if (sscanf(comando + 7, "%29s %3s %255s", colName, op, value) != 3) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: filter <nombre_columna> eq/neq/gt/lt <valor>\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            filterDataframe(dfActivo, colName, op, value);

        } else if (strncmp(comando, "delnull", 7) == 0) {
            // Comando "delnull": eliminar filas con valores nulos
            char colName[30];
            if (sscanf(comando + 8, "%29s", colName) != 1) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: delnull <nombre_columna>\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            delnull(dfActivo, colName);

        } else if (strncmp(comando, "delcolum", 8) == 0) {
            // Comando "delcolum": eliminar columna del dataframe activo
            char colName[30];
            if (sscanf(comando + 9, "%29s", colName) != 1) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: delcolum <nombre_columna>\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            delcolum(dfActivo, colName);

        } else if (strncmp(comando, "quarter", 7) == 0) {
            // Comando "quarter": generar nueva columna de trimestre
            char colName[30], newColName[30];
            if (sscanf(comando + 8, "%29s %29s", colName, newColName) != 2) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: quarter <nombre_columna> <nombre_nueva_columna>\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            quarter(dfActivo, colName, newColName);
            
        } else if (strncmp(comando, "sort", 4) == 0) {
            char colName[30], order[4] = "asc";
            int params = sscanf(comando + 5, "%29s %3s", colName, order);

            if (params < 1 || (strcmp(order, "asc") != 0 && strcmp(order, "des") != 0)) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: sort <nombre_columna> [asc/des]\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            sortDataframe(dfActivo, colName, (params == 2) ? order : "asc");

        } else if (strncmp(comando, "name", 4) == 0) {
            // Comando "name": renombrar el dataframe activo
            char nuevoNombre[51];
            if (sscanf(comando + 5, "%50s", nuevoNombre) != 1) {
                TextColor(RED);
                printf("Error: Debe especificar un nombre.\n");
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                continue;
            }

            if (renombrarDataframe(dfActivo, nuevoNombre, &listaDeDataframes)) {
                TextColor(GREEN);
                printf("Nombre cambiado a '%s'.\n", dfActivo->nombre);
                TextColor(WHITE);
            }

        } else if (strncmp(comando, "list", 4) == 0) {
            // Comando "list": listar dataframes cargados
            if (strlen(comando) > 4) {
                TextColor(RED);
                printf("Error: El comando 'list' no lleva parametros.\n");
            } else {
                listarDataframes(&listaDeDataframes);
            }

        } else if (strncmp(comando, "prefix", 6) == 0) {
            // Comando "prefix": generar columna con prefijo
            char colName[30], newColName[30];
            int n;
            if (sscanf(comando + 7, "%29s %d %29s", colName, &n, newColName) != 3) {
                TextColor(RED);
                printf("Error: Uso incorrecto del comando. Sintaxis: prefix <columna> n <nueva_columna>\n");
                TextColor(WHITE);
                continue;
            }

            if (!dfActivo) {
                TextColor(RED);
                printf("Error: No hay un dataframe activo.\n");
                TextColor(WHITE);
                continue;
            }

            prefixColumna(dfActivo, colName, n, newColName);

        } else {
            // Comando no reconocido
            TextColor(RED);
            printf("Comando no reconocido.\n");
        }
    }

    return 0;
}

