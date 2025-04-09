# 🧠 Shell interactivo de Dataframes en C

Este proyecto implementa una aplicación de consola en C que permite la carga, visualización, modificación y exportación de estructuras tipo `dataframe`, similares a las de bibliotecas como pandas, pero usando C puro y memoria dinámica.

## 📋 Funcionalidades principales

- Cargar archivos `.csv` como dataframes
- Visualizar contenido con `view [n]`
- Mostrar metadatos (`meta`)
- Filtrado de filas (`filter <col> eq/neq/gt/lt <valor>`)
- Eliminar filas con nulos o columnas completas (`delnull`, `delcolum`)
- Ordenar columnas (`sort <col> asc|des`)
- Guardar en archivo (`save <archivo.csv>`)
- Generar columnas derivadas (`quarter`, `prefix`)
- Renombrar y listar dataframes (`name`, `list`)
- Salida del programa con limpieza de memoria (`quit`)

## 🧪 Estructura de archivos

- `main.c`: bucle interactivo de comandos, menú y control de flujos
- `lib.c`: implementación de operaciones sobre los dataframes
- `lib.h`: estructuras, definiciones de colores y prototipos

## 💻 Compilación

Puedes compilarlo desde terminal con:

```bash
gcc main.c lib.c -o dataframe-shell
```

Luego ejecuta:

```bash
./dataframe-shell
```

## 🧑‍💻 Autor

Alejandro Gallego López  
Contacto: alejandro.gallego@goumh.umh.es