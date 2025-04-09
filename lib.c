#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

// Variables globales
int i, j, row, col;
char *token;
char line[1024];
int anio, mes, dia;
int numColumns = 0;
int numRows = 0;

// Funcion para cambiar el color del texto
void TextColor(int color) {
    static int __BACKGROUND;

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

    GetConsoleScreenBufferInfo(h, &csbiInfo);
    __BACKGROUND = csbiInfo.wAttributes >> 4;
    SetConsoleTextAttribute(h, color + (__BACKGROUND << 4));
}

void freeDataframe(Dataframe *df) {
    if (!df) return; // Evitar liberar un dataframe nulo

    for (int i = 0; i < df->numColumnas; i++) {
        Columna *columna = &df->columnas[i];

        // Liberar datos almacenados en la columna
        if (columna->datos) {
            if (columna->tipo == TEXTO) {
                for (int j = 0; j < columna->numFilas; j++) {
                    if (((char **)columna->datos)[j]) {
                        free(((char **)columna->datos)[j]);
                    }
                }
            }
            free(columna->datos);
        }

        // Liberar el array que gestiona los valores nulos
        free(columna->esNulo);
    }

    // Liberar el array de columnas y el índice de filas
    free(df->columnas);
    free(df->indice);

    // Finalmente, liberar la estructura del dataframe
    free(df);
}



const char* obtenerNombreTipo(int tipo) {
    switch (tipo) {
        case NUMERICO:
            return "NUMERICO";
        case TEXTO:
            return "TEXTO";
        case FECHA:
            return "FECHA";
        default:
            return "DESCONOCIDO";
    }
}

void metaDataframe(const Dataframe *df) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    TextColor(GREEN);
    printf("Metadatos del dataframe:\n");
    for (int i = 0; i < df->numColumnas; i++) {
        if (!df->columnas[i].esNulo) {
            TextColor(RED);
            printf("Error critico: La columna '%s' no tiene informacion sobre valores nulos.\n", df->columnas[i].nombre);
            TextColor(WHITE);
            continue;
        }

        int nulos = 0;
        for (int j = 0; j < df->numFilas; j++) {
            if (df->columnas[i].esNulo[j]) {
                nulos++;
            }
        }

        printf("Columna: %s\n", df->columnas[i].nombre);
        printf(" - Tipo: %s\n", obtenerNombreTipo(df->columnas[i].tipo));
        printf(" - Valores nulos: %d\n", nulos);
    }
    TextColor(WHITE);
}

void viewDataframe(const Dataframe *df, int n) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    if (n == 0) {
        n = (df->numFilas < 10) ? df->numFilas : 10;
    } else if (abs(n) > df->numFilas) {
        n = df->numFilas;
    }

    TextColor(LIGHTGREEN);

    // Imprimir nombres de columnas
    for (int i = 0; i < df->numColumnas; i++) {
        printf("%-15s", df->columnas[i].nombre);
    }
    printf("\n");

    // Mostrar las filas
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            int actualRow = df->indice[i];
            for (int col = 0; col < df->numColumnas; col++) {
                if (df->columnas[col].esNulo[actualRow]) {
                    printf("%-15s", "#N/A");
                } else {
                    switch (df->columnas[col].tipo) {
                        case NUMERICO:
                            printf("%-15.2f", ((double *)df->columnas[col].datos)[actualRow]);
                            break;
                        case FECHA: {
                            Fecha *fecha = &((Fecha *)df->columnas[col].datos)[actualRow];
                            if (fecha->tm_year >= 0 && fecha->tm_mon >= 0 && fecha->tm_mday > 0) {
                                char buffer[20];
                                sprintf(buffer, "%04d-%02d-%02d", fecha->tm_year + 1900, fecha->tm_mon + 1, fecha->tm_mday);
                                printf("%-15s", buffer);
                            } else {
                                printf("%-15s", "#N/A");
                            }
                            break;
                        }
                        case TEXTO: {
                            char *text = ((char **)df->columnas[col].datos)[actualRow];
                            if (text && text[0] != '\0') {
                                printf("%-15s", text);
                            } else {
                                printf("%-15s", "#N/A");
                            }
                            break;
                        }
                    }
                }
            }
            printf("\n");
        }
    } else {
        n = abs(n);
        for (int i = df->numFilas - 1; i >= df->numFilas - n; i--) {
            int actualRow = df->indice[i];
            for (int col = 0; col < df->numColumnas; col++) {
                if (df->columnas[col].esNulo[actualRow]) {
                    printf("%-15s", "#N/A");
                } else {
                    switch (df->columnas[col].tipo) {
                        case NUMERICO:
                            printf("%-15.2f", ((double *)df->columnas[col].datos)[actualRow]);
                            break;
                        case FECHA: {
                            Fecha *fecha = &((Fecha *)df->columnas[col].datos)[actualRow];
                            if (fecha->tm_year >= 0 && fecha->tm_mon >= 0 && fecha->tm_mday > 0) {
                                char buffer[20];
                                sprintf(buffer, "%04d-%02d-%02d", fecha->tm_year + 1900, fecha->tm_mon + 1, fecha->tm_mday);
                                printf("%-15s", buffer);
                            } else {
                                printf("%-15s", "#N/A");
                            }
                            break;
                        }
                        case TEXTO:
                            printf("%-15s", ((char **)df->columnas[col].datos)[actualRow]);
                            break;
                    }
                }
            }
            printf("\n");
        }
    }

    TextColor(WHITE);
}




int validarFecha(const char *comando, int *anio, int *mes, int *dia) {
    if (strlen(comando) != 10) {
        return 0; // Fecha incorrecta
    }

    // Verificar separadores
    if ((comando[4] != '/' && comando[4] != '-') || comando[4] != comando[7]) {
        return 0; // Fecha incorrecta
    }

    // Verificar que todos los caracteres sean validos
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (comando[i] < '0' || comando[i] > '9') {
            return 0; // Fecha incorrecta
        }
    }

    // Extraer valores de dia, mes y año
    int d = (comando[8] - '0') * 10 + (comando[9] - '0');
    int m = (comando[5] - '0') * 10 + (comando[6] - '0');
    int a = (comando[0] - '0') * 1000 + (comando[1] - '0') * 100 + (comando[2] - '0') * 10 + (comando[3] - '0');

    // Validar rango de fecha
    if (a < 1 || m < 1 || m > 12 || d < 1 || d > 31) return 0;
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30) return 0;
    if (m == 2) {
        if (a % 400 == 0 || (a % 4 == 0 && a % 100 != 0)) { // Anio bisiesto
            if (d > 29) return 0;
        } else {
            if (d > 28) return 0;
        }
    }

    // Asignar valores si los punteros no son NULL
    if (anio) *anio = a;
    if (mes) *mes = m;
    if (dia) *dia = d;

    return 1; // Fecha valida
}


void filterDataframe(Dataframe *df, const char *colName, const char *op, const char *value) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, colName) == 0) {
            colIndex = i;
            break;
        }
    }

    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe en el dataframe.\n", colName);
        TextColor(WHITE);
        return;
    }

    Columna *columnaFiltro = &df->columnas[colIndex];
    int *filasValidas = calloc(df->numFilas, sizeof(int));
    int validRowCount = 0;

    for (int row = 0; row < df->numFilas; row++) {
        if (columnaFiltro->esNulo[row]) continue;

        int conditionMet = 0;
        if (columnaFiltro->tipo == NUMERICO) {
            double cellValue = ((double *)columnaFiltro->datos)[row];
            double filterValue = atof(value);
            if (strcmp(op, "eq") == 0) conditionMet = (cellValue == filterValue);
            else if (strcmp(op, "lt") == 0) conditionMet = (cellValue < filterValue);
            else if (strcmp(op, "gt") == 0) conditionMet = (cellValue > filterValue);
        }

        if (conditionMet) {
            filasValidas[row] = 1;
            validRowCount++;
        }
    }

    for (int col = 0; col < df->numColumnas; col++) {
        Columna *columna = &df->columnas[col];
        void *newData = malloc(validRowCount * (columna->tipo == NUMERICO ? sizeof(double) : 
                                                (columna->tipo == FECHA ? sizeof(Fecha) : sizeof(char *))));
        unsigned char *newEsNulo = calloc(validRowCount, sizeof(unsigned char));

        int newRow = 0;
        for (int row = 0; row < df->numFilas; row++) {
            if (!filasValidas[row]) continue;

            if (columna->tipo == NUMERICO) {
                ((double *)newData)[newRow] = ((double *)columna->datos)[row];
            } else if (columna->tipo == FECHA) {
                ((Fecha *)newData)[newRow] = ((Fecha *)columna->datos)[row];
            } else if (columna->tipo == TEXTO) {
                ((char **)newData)[newRow] = strdup(((char **)columna->datos)[row]);
            }
            newEsNulo[newRow] = columna->esNulo[row];
            newRow++;
        }

        free(columna->datos);
        free(columna->esNulo);
        columna->datos = newData;
        columna->esNulo = newEsNulo;
    }

    free(filasValidas);
    df->numFilas = validRowCount;

    TextColor(GREEN);
    printf("Filtro aplicado. Filas restantes: %d\n", df->numFilas);
    TextColor(WHITE);
}

void auxLoad(const char *line, char sep, char **fields, int *num_fields) {
    char *field = (char *)malloc(strlen(line) + 1); // Buffer temporal para el campo actual
    if (!field) {
        perror("Error al reservar memoria para campo");
        exit(EXIT_FAILURE);
    }

    int field_pos = 0;    // Posición actual dentro del campo
    int field_count = 0;  // Número de campos encontrados

    for (const char *c = line; *c != '\0'; c++) {
        if (*c == sep) {
            // Fin de un campo: cerrar el buffer actual
            field[field_pos] = '\0';
            fields[field_count] = strdup(field);
            if (!fields[field_count]) {
                perror("Error al reservar memoria para un campo");
                exit(EXIT_FAILURE);
            }
            field_count++;
            field_pos = 0; // Reiniciar posición para el siguiente campo
        } else if (*c != '\r' && *c != '\n') {
            // Agregar carácter válido al buffer (ignorar saltos de línea)
            field[field_pos++] = *c;
        }
    }

    // Manejar el último campo de la línea
    field[field_pos] = '\0';
    fields[field_count] = strdup(field);
    if (!fields[field_count]) {
        perror("Error al reservar memoria para el ultimo campo");
        exit(EXIT_FAILURE);
    }
    field_count++;

    // Actualizar el número total de campos encontrados
    *num_fields = field_count;

    // Liberar memoria temporal
    free(field);
}



void saveDataframe(const Dataframe *df, const char *filename) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe activo.\n");
        TextColor(WHITE);
        return;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        TextColor(RED);
        printf("Error: No se pudo crear el archivo '%s'.\n", filename);
        TextColor(WHITE);
        return;
    }

    // Escribir nombres de columnas
    for (int col = 0; col < df->numColumnas; col++) {
        fprintf(file, "%s%s", df->columnas[col].nombre, col < df->numColumnas - 1 ? "," : "\n");
    }

    // Escribir datos del dataframe
    for (int row = 0; row < df->numFilas; row++) {
        for (int col = 0; col < df->numColumnas; col++) {
            Columna *column = &df->columnas[col];
            if (column->esNulo[row]) {
                fprintf(file, "%s", "#N/A");
            } else if (column->tipo == NUMERICO) {
                fprintf(file, "%.2f", ((double *)column->datos)[row]);
            } else if (column->tipo == FECHA) {
                Fecha *fecha = &((Fecha *)column->datos)[row];
                fprintf(file, "%04d-%02d-%02d", fecha->tm_year + 1900, fecha->tm_mon + 1, fecha->tm_mday);
            } else { // TEXTO
                fprintf(file, "%s", ((char **)column->datos)[row]);
            }
            fprintf(file, "%s", col < df->numColumnas - 1 ? "," : "\n");
        }
    }

    fclose(file);

    TextColor(GREEN);
    printf("Dataframe guardado exitosamente en '%s'.\n", filename);
    TextColor(WHITE);
}

void delnull(Dataframe *df, const char *colName) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    // Buscar indice de la columna
    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, colName) == 0) {
            colIndex = i;
            break;
        }
    }

    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe en el dataframe.\n", colName);
        TextColor(WHITE);
        return;
    }

    Columna *columnaFiltro = &df->columnas[colIndex];
    int validRowCount = 0;

    // Contar filas validas
    for (int i = 0; i < df->numFilas; i++) {
        if (!columnaFiltro->esNulo[i]) {
            validRowCount++;
        }
    }

    if (validRowCount == 0) {
        TextColor(YELLOW);
        printf("Todas las filas contienen valores nulos en la columna '%s'.\n", colName);
        TextColor(WHITE);
        return;
    }

    // Crear nuevas estructuras para almacenar datos validos
    for (int col = 0; col < df->numColumnas; col++) {
        Columna *columna = &df->columnas[col];
        void *newData = malloc(validRowCount * (columna->tipo == NUMERICO ? sizeof(double) :
                                                (columna->tipo == FECHA ? sizeof(Fecha) : sizeof(char *))));
        unsigned char *newEsNulo = calloc(validRowCount, sizeof(unsigned char));

        if (!newData || !newEsNulo) {
            perror("Error al asignar memoria para nueva columna");
            exit(EXIT_FAILURE);
        }

        int newRow = 0;
        for (int row = 0; row < df->numFilas; row++) {
            if (!columnaFiltro->esNulo[row]) {
                if (columna->tipo == NUMERICO) {
                    ((double *)newData)[newRow] = ((double *)columna->datos)[row];
                } else if (columna->tipo == FECHA) {
                    ((Fecha *)newData)[newRow] = ((Fecha *)columna->datos)[row];
                } else if (columna->tipo == TEXTO) {
                    if (((char **)columna->datos)[row] != NULL) {
                        ((char **)newData)[newRow] = strdup(((char **)columna->datos)[row]);
                        if (!((char **)newData)[newRow]) {
                            perror("Error al duplicar cadena");
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        ((char **)newData)[newRow] = NULL;
                    }
                }
                newEsNulo[newRow] = columna->esNulo[row];
                newRow++;
            } else if (columna->tipo == TEXTO && ((char **)columna->datos)[row]) {
                free(((char **)columna->datos)[row]);
                ((char **)columna->datos)[row] = NULL;
            }
        }

        free(columna->datos);
        free(columna->esNulo);
        columna->datos = newData;
        columna->esNulo = newEsNulo;
    }

    df->numFilas = validRowCount;

    TextColor(GREEN);
    printf("Valores nulos eliminados en la columna '%s'. Filas restantes: %d\n", colName, df->numFilas);
    TextColor(WHITE);
}

void delcolum(Dataframe *df, const char *colName) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    // Buscar índice de la columna
    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, colName) == 0) {
            colIndex = i;
            break;
        }
    }

    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe en el dataframe.\n", colName);
        TextColor(WHITE);
        return;
    }

    Columna *columna = &df->columnas[colIndex];
    if (columna->datos) {
        if (columna->tipo == TEXTO) {
            for (int i = 0; i < df->numFilas; i++) {
                free(((char **)columna->datos)[i]);
            }
        }
        free(columna->datos);
    }
    free(columna->esNulo);

    // Reorganizar columnas
    for (int i = colIndex; i < df->numColumnas - 1; i++) {
        df->columnas[i] = df->columnas[i + 1];
    }

    // Ajustar tamaño del array
    Columna *temp = realloc(df->columnas, (df->numColumnas - 1) * sizeof(Columna));
    if (!temp && df->numColumnas > 1) {
        perror("Error al reasignar memoria para columnas");
        exit(EXIT_FAILURE);
    }
    df->columnas = temp;
    df->numColumnas--;
}



void quarter(Dataframe *df, const char *colName, const char *newColName) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe cargado.\n");
        TextColor(WHITE);
        return;
    }

    // Buscar indice de la columna base
    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, colName) == 0) {
            colIndex = i;
            break;
        }
    }

    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe.\n", colName);
        TextColor(WHITE);
        return;
    }

    // Verificar que la columna es de tipo FECHA
    if (df->columnas[colIndex].tipo != FECHA) {
        TextColor(RED);
        printf("Error: La columna '%s' no es de tipo FECHA.\n", colName);
        TextColor(WHITE);
        return;
    }

    // Verificar que el nombre de la nueva columna no exista
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, newColName) == 0) {
            TextColor(RED);
            printf("Error: Ya existe una columna con el nombre '%s'.\n", newColName);
            TextColor(WHITE);
            return;
        }
    }

    // Insertar la nueva columna
    df->columnas = realloc(df->columnas, (df->numColumnas + 1) * sizeof(Columna));
    if (!df->columnas) {
        perror("Error al reasignar memoria para columnas");
        exit(EXIT_FAILURE);
    }

    // Mover columnas a la derecha para insertar en la posicion correcta
    for (int i = df->numColumnas; i > colIndex + 1; i--) {
        df->columnas[i] = df->columnas[i - 1];
    }

    // Inicializar la nueva columna
    Columna *newCol = &df->columnas[colIndex + 1];
    strncpy(newCol->nombre, newColName, sizeof(newCol->nombre) - 1);
    newCol->tipo = TEXTO;
    newCol->numFilas = df->numFilas;
    newCol->datos = malloc(df->numFilas * sizeof(char *));
    newCol->esNulo = calloc(df->numFilas, sizeof(unsigned char));

    if (!newCol->datos || !newCol->esNulo) {
        perror("Error al asignar memoria para nueva columna");
        exit(EXIT_FAILURE);
    }

    // Asignar valores a la nueva columna
    Columna *baseCol = &df->columnas[colIndex];
    for (int i = 0; i < df->numFilas; i++) {
        if (baseCol->esNulo[i]) {
            newCol->esNulo[i] = 1;
            ((char **)newCol->datos)[i] = strdup("#N/A");
        } else {
            Fecha *fecha = &((Fecha *)baseCol->datos)[i];
            int mes = fecha->tm_mon + 1;
            const char *valor = (mes >= 1 && mes <= 3)  ? "Q1" :
                                (mes >= 4 && mes <= 6)  ? "Q2" :
                                (mes >= 7 && mes <= 9)  ? "Q3" :
                                (mes >= 10 && mes <= 12) ? "Q4" : "#N/A";
            ((char **)newCol->datos)[i] = strdup(valor);
        }
    }

    df->numColumnas++;
}


// Renombrar un dataframe activo
int renombrarDataframe(Dataframe *df, const char *nuevoNombre, Lista *lista) {
    if (strlen(nuevoNombre) > 50) {
        TextColor(RED);
        printf("Error: El nombre excede los 50 caracteres.\n");
        TextColor(WHITE);
        return 0;
    }

    Nodo *nodoActual = lista->primero;
    while (nodoActual) {
        if (strcmp(nodoActual->df->nombre, nuevoNombre) == 0) {
            TextColor(RED);
            printf("Error: Ya existe un dataframe con ese nombre.\n");
            TextColor(WHITE);
            return 0;
        }
        nodoActual = nodoActual->siguiente;
    }

    strcpy(df->nombre, nuevoNombre);
    return 1;
}

void listarDataframes(const Lista *lista) {
    if (!lista || lista->numDFs == 0) {
        TextColor(RED);
        printf("No hay dataframes cargados.\n");
        TextColor(WHITE);
        return;
    }

    TextColor(GREEN);
    printf("Listado de dataframes:\n");
    Nodo *actual = lista->primero;
    while (actual) {
        Dataframe *df = actual->df;
        printf("%s: %d filas, %d columnas\n", df->nombre, df->numFilas, df->numColumnas);
        actual = actual->siguiente;
    }
    TextColor(WHITE);
}

void prefixColumna(Dataframe *df, const char *columna, int n, const char *nuevaColumna) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe activo.\n");
        TextColor(WHITE);
        return;
    }

    // Validar que 'n' es mayor que 0
    if (n <= 0) {
        TextColor(RED);
        printf("Error: El valor de 'n' debe ser mayor que 0.\n");
        TextColor(WHITE);
        return;
    }

    // Verificar que la columna existe y es de tipo TEXTO
    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, columna) == 0) {
            colIndex = i;
            break;
        }
    }
    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe.\n", columna);
        TextColor(WHITE);
        return;
    }
    if (df->columnas[colIndex].tipo != TEXTO) {
        TextColor(RED);
        printf("Error: La columna '%s' no es de tipo texto.\n", columna);
        TextColor(WHITE);
        return;
    }

    // Verificar que el nuevo nombre no existe
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, nuevaColumna) == 0) {
            TextColor(RED);
            printf("Error: El nombre '%s' ya existe como columna.\n", nuevaColumna);
            TextColor(WHITE);
            return;
        }
    }

    // Crear la nueva columna
    Columna nuevaCol;
    strncpy(nuevaCol.nombre, nuevaColumna, sizeof(nuevaCol.nombre) - 1);
    nuevaCol.nombre[sizeof(nuevaCol.nombre) - 1] = '\0';
    nuevaCol.tipo = TEXTO;
    nuevaCol.numFilas = df->numFilas;

    // Asignar memoria para los datos y valores nulos
    nuevaCol.datos = malloc(df->numFilas * sizeof(char *));
    nuevaCol.esNulo = calloc(df->numFilas, sizeof(unsigned char));

    char **originalData = (char **)df->columnas[colIndex].datos;
    char **newData = (char **)nuevaCol.datos;

    for (int i = 0; i < df->numFilas; i++) {
        if (!df->columnas[colIndex].esNulo[i]) {
            size_t len = strlen(originalData[i]);
            size_t prefixLen = len < n ? len : n; // Determinar cuantos caracteres copiar
            newData[i] = malloc(prefixLen + 1);  // Reservar memoria para la nueva cadena
            if (!newData[i]) {
                perror("Error al asignar memoria para prefijo");
                exit(EXIT_FAILURE);
            }
            strncpy(newData[i], originalData[i], prefixLen); // Copiar los primeros 'prefixLen' caracteres
            newData[i][prefixLen] = '\0'; // Asegurar el terminador nulo
        } else {
            nuevaCol.esNulo[i] = 1;
            newData[i] = NULL;
        }
    }

    // Agregar la nueva columna al dataframe
    df->columnas = realloc(df->columnas, (df->numColumnas + 1) * sizeof(Columna));
    if (!df->columnas) {
        perror("Error al redimensionar columnas del dataframe");
        exit(EXIT_FAILURE);
    }
    df->columnas[df->numColumnas] = nuevaCol;
    df->numColumnas++;

    TextColor(GREEN);
    printf("Columna '%s' creada con prefijos de '%s' de longitud %d.\n", nuevaColumna, columna, n);
    TextColor(WHITE);
}

void sortDataframe(Dataframe *df, const char *colName, const char *order) {
    if (!df) {
        TextColor(RED);
        printf("Error: No hay un dataframe activo.\n");
        TextColor(WHITE);
        return;
    }

    // Buscar el indice de la columna
    int colIndex = -1;
    for (int i = 0; i < df->numColumnas; i++) {
        if (strcmp(df->columnas[i].nombre, colName) == 0) {
            colIndex = i;
            break;
        }
    }
    if (colIndex == -1) {
        TextColor(RED);
        printf("Error: La columna '%s' no existe en el dataframe.\n", colName);
        TextColor(WHITE);
        return;
    }

    Columna *columna = &df->columnas[colIndex];
    int ascendente = (order == NULL || strcmp(order, "asc") == 0);

    // Verificar que los punteros de la columna son validos
    if (!columna->datos || !columna->esNulo) {
        TextColor(RED);
        printf("Error: La columna '%s' tiene datos o valores nulos invalidos.\n", colName);
        TextColor(WHITE);
        return;
    }

    // Ordenacion utilizando el array indice
    for (int i = 0; i < df->numFilas - 1; i++) {
        for (int j = i + 1; j < df->numFilas; j++) {
            int swap = 0;
            int isNull1 = columna->esNulo[df->indice[i]];
            int isNull2 = columna->esNulo[df->indice[j]];

            // Manejo de nulos
            if (isNull1 && !isNull2) {
                swap = ascendente ? -1 : 1;
            } else if (!isNull1 && isNull2) {
                swap = ascendente ? 1 : -1;
            } else if (!isNull1 && !isNull2) {
                // Comparar valores no nulos segun el tipo
                if (columna->tipo == NUMERICO) {
                    double val1 = ((double *)columna->datos)[df->indice[i]];
                    double val2 = ((double *)columna->datos)[df->indice[j]];
                    swap = (val1 > val2) - (val1 < val2);
                } else if (columna->tipo == TEXTO) {
                    char *val1 = ((char **)columna->datos)[df->indice[i]];
                    char *val2 = ((char **)columna->datos)[df->indice[j]];
                    swap = strcmp(val1, val2);
                } else if (columna->tipo == FECHA) {
                    Fecha *val1 = &((Fecha *)columna->datos)[df->indice[i]];
                    Fecha *val2 = &((Fecha *)columna->datos)[df->indice[j]];
                    if (val1->tm_year != val2->tm_year)
                        swap = val1->tm_year - val2->tm_year;
                    else if (val1->tm_mon != val2->tm_mon)
                        swap = val1->tm_mon - val2->tm_mon;
                    else
                        swap = val1->tm_mday - val2->tm_mday;
                }
                swap = ascendente ? swap : -swap;
            }

            // Intercambiar indices si es necesario
            if (swap > 0) {
                int temp = df->indice[i];
                df->indice[i] = df->indice[j];
                df->indice[j] = temp;
            }
        }
    }

    TextColor(GREEN);
    printf("Dataframe ordenado por la columna '%s' en orden %s.\n", colName, ascendente ? "ascendente" : "descendente");
    TextColor(WHITE);
}

Dataframe* Load(const char *nomFichero, char sep, int cab) {
    FILE *file = fopen(nomFichero, "r");
    if (!file) {
        TextColor(RED);
        printf("Error: No se pudo abrir el fichero '%s'.\n", nomFichero);
        TextColor(WHITE);
        return NULL;
    }

    char line[1024];
    char *fields[100];
    int num_fields = 0;
    int i;

    // Leer la cabecera
    if (!fgets(line, sizeof(line), file)) {
        TextColor(RED);
        printf("Error: El fichero esta vacio o no se pudo leer.\n");
        TextColor(WHITE);
        fclose(file);
        return NULL;
    }

    auxLoad(line, sep, fields, &num_fields);

    // Crear el dataframe
    Dataframe *df = (Dataframe *)malloc(sizeof(Dataframe));
    if (!df) {
        TextColor(RED);
        printf("Error: Fallo al reservar memoria para el dataframe.\n");
        TextColor(WHITE);
        fclose(file);
        return NULL;
    }

    df->columnas = (Columna *)malloc(num_fields * sizeof(Columna));
    if (!df->columnas) {
        TextColor(RED);
        printf("Error: Fallo al reservar memoria para las columnas.\n");
        TextColor(WHITE);
        free(df);
        fclose(file);
        return NULL;
    }

    for (i = 0; i < num_fields; i++) {
        strncpy(df->columnas[i].nombre, fields[i], sizeof(df->columnas[i].nombre) - 1);
        free(fields[i]);
        df->columnas[i].datos = NULL;
        df->columnas[i].esNulo = NULL;
        df->columnas[i].numFilas = 0;
        df->columnas[i].tipo = TEXTO; // Tipo inicial predeterminado
    }

    df->numColumnas = num_fields;
    df->numFilas = 0;
    df->indice = NULL;

    // Leer filas de datos y detectar tipos de columnas
    while (fgets(line, sizeof(line), file)) {
        auxLoad(line, sep, fields, &num_fields);

        if (num_fields != df->numColumnas) {
            TextColor(RED);
            printf("Error: El numero de columnas no coincide con la cabecera.\n");
            TextColor(WHITE);
            for (i = 0; i < num_fields; i++) free(fields[i]);
            freeDataframe(df);
            fclose(file);
            return NULL;
        }

        for (i = 0; i < df->numColumnas; i++) {
            df->columnas[i].esNulo = realloc(df->columnas[i].esNulo, (df->numFilas + 1) * sizeof(unsigned char));
            if (!df->columnas[i].esNulo) {
                TextColor(RED);
                printf("Error: Fallo al reservar memoria para valores nulos en la columna '%s'.\n", df->columnas[i].nombre);
                TextColor(WHITE);
                freeDataframe(df);
                fclose(file);
                return NULL;
            }

            if (fields[i][0] == '\0') { // Valor nulo
                df->columnas[i].esNulo[df->numFilas] = 1;
            } else {
                df->columnas[i].esNulo[df->numFilas] = 0;

                if (df->columnas[i].tipo == TEXTO) {
                    // Verificar si es posible que sea NUMERICO o FECHA
                    char *endptr;
                    strtod(fields[i], &endptr);
                    if (*endptr == '\0') { // Si es numerico
                        df->columnas[i].tipo = NUMERICO;
                    } else if (validarFecha(fields[i], NULL, NULL, NULL)) { // Si es fecha
                        df->columnas[i].tipo = FECHA;
                    }
                }

                // Asignar datos segun el tipo detectado
                if (df->columnas[i].tipo == NUMERICO) { // Asignar numeros
                    df->columnas[i].datos = realloc(df->columnas[i].datos, (df->numFilas + 1) * sizeof(double));
                    if (!df->columnas[i].datos) {
                        TextColor(RED);
                        printf("Error: Fallo al reservar memoria para datos numericos en la columna '%s'.\n", df->columnas[i].nombre);
                        TextColor(WHITE);
                        freeDataframe(df);
                        fclose(file);
                        return NULL;
                    }
                    ((double *)df->columnas[i].datos)[df->numFilas] = atof(fields[i]);
                } else if (df->columnas[i].tipo == FECHA) { // Asignar fechas
                    int anio, mes, dia;
                    if (validarFecha(fields[i], &anio, &mes, &dia)) {
                        df->columnas[i].datos = realloc(df->columnas[i].datos, (df->numFilas + 1) * sizeof(Fecha));
                        if (!df->columnas[i].datos) {
                            TextColor(RED);
                            printf("Error: Fallo al reservar memoria para datos de fecha en la columna '%s'.\n", df->columnas[i].nombre);
                            TextColor(WHITE);
                            freeDataframe(df);
                            fclose(file);
                            return NULL;
                        }
                        Fecha *fecha = &((Fecha *)df->columnas[i].datos)[df->numFilas];
                        fecha->tm_year = anio - 1900;
                        fecha->tm_mon = mes - 1;
                        fecha->tm_mday = dia;
                    }
                } else { // Asignar texto
                    df->columnas[i].datos = realloc(df->columnas[i].datos, (df->numFilas + 1) * sizeof(char *));
                    if (!df->columnas[i].datos) {
                        TextColor(RED);
                        printf("Error: Fallo al reservar memoria para datos de texto en la columna '%s'.\n", df->columnas[i].nombre);
                        TextColor(WHITE);
                        freeDataframe(df);
                        fclose(file);
                        return NULL;
                    }
                    ((char **)df->columnas[i].datos)[df->numFilas] = strdup(fields[i]);
                }
            }
            free(fields[i]); // Liberar memoria de la celda procesada
        }

        df->numFilas++;
    }

    // Inicializar el array indice
    df->indice = (int *)malloc(df->numFilas * sizeof(int));
    if (!df->indice) {
        TextColor(RED);
        printf("Error: Fallo al asignar memoria para el array de indices.\n");
        TextColor(WHITE);
        freeDataframe(df);
        fclose(file);
        return NULL;
    }
    for (int i = 0; i < df->numFilas; i++) {
        df->indice[i] = i; // indices en orden inicial
    }

    TextColor(GREEN);
    printf("Dataframe cargado exitosamente con %d filas y %d columnas.\n", df->numFilas, df->numColumnas);
    TextColor(WHITE);

    fclose(file);
    return df;
}


