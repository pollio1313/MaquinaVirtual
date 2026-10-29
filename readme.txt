
### 1. ¿Por qué `#ifndef`, `#define` y `#endif` en los `.h`?

* **Función:** Se llaman *guardas de inclusión*.
* **Por qué hay que dejarlos:** Cuando varios archivos `.c` incluyen el mismo `.h` (o cuando un `.h` incluye a otro), el compilador intenta procesar las estructuras dos veces. Si no están estas líneas, la compilación se interrumpe con el error `redefinition of struct...`. Esas directivas aseguran que el archivo solo se lea una vez.

### 2. ¿Qué hace `extern`?

* **Función:** Avisa que una variable global existe y está creada en otro archivo `.c`.


* **Por qué hay que dejarlo:** Si dos archivos `.c` declaran la misma variable global (por ejemplo `int gTamMemoriaBytes;`), el compilador no sabe cuál usar y da error de duplicación. Con `extern int gTamMemoriaBytes;` en el header, le decimos a `MV0.c`: *"esta variable ya existe y tiene memoria asignada adentro de `imagen.c`, usá esa misma"*.



### 3. ¿Por qué separar `imagen.c` en vez de meter todo en `instrucciones.c`?

* **Principio de responsabilidad única:** `instrucciones.c` se encarga exclusivamente del repertorio de la CPU y la ejecución de código. El módulo `imagen.c` resuelve la persistencia en disco y el formateo de archivos `.vmi` para el debugger.


* **Trabajo en equipo y control de cambios:** Al estar desacoplado, cualquiera puede corregir la serialización del `.vmi` o los breakpoints sin tocar ni arriesgarse a romper la lógica de los saltos, operaciones lógicas o direccionamiento de memoria de la CPU.