#!/bin/bash
# Corre todas las pruebas de esta carpeta contra la maquina virtual.
# Uso (desde Git Bash, dentro de esta carpeta):   ./correr_pruebas.sh
# Busca vmt y vmx en el PATH, en esta carpeta o en la carpeta de arriba.
# Se pueden indicar a mano:  VMT=../vmt.exe VMX=../vmx.exe ./correr_pruebas.sh
cd "$(dirname "$0")" || exit 1

buscar() {
    for c in "$1" "./$1" "../$1" "./$1.exe" "../$1.exe"; do
        if command -v "$c" >/dev/null 2>&1 || [ -x "$c" ]; then echo "$c"; return; fi
    done
}
VMT=${VMT:-$(buscar vmt)}
VMX=${VMX:-$(buscar vmx)}
[ -z "$VMX" ] && { echo "No encuentro vmx (compilalo: gcc -o vmx MV0.c imagen.c instrucciones.c)"; exit 1; }

ok=0; mal=0
falla() { echo "  FALLA: $1"; mal=$((mal+1)); }

for esp in *.esperado *.error; do
    [ -e "$esp" ] || continue
    base="${esp%.*}"
    # 1) traducir si hay .asm
    if [ -f "$base.asm" ]; then
        [ -z "$VMT" ] && { echo "[$base] sin vmt: se omite"; continue; }
        "$VMT" "$base.asm" >/dev/null 2>&1
    fi
    [ -f "$base.vmx" ] || { echo "[$base] falta $base.vmx"; mal=$((mal+1)); continue; }

    # 2) ejecutar (con entrada si existe $base.in)
    entrada=/dev/null; [ -f "$base.in" ] && entrada="$base.in"
    # se ignoran diferencias de espacios (repetidos o al final de linea): se comparan los valores
    "$VMX" "$base.vmx" < "$entrada" 2>&1 | tr -d '\r' | sed 's/[[:space:]][[:space:]]*/ /g; s/ $//' > "$base.salida"
    rc=${PIPESTATUS[0]}

    # 3) comparar
    echo "[$base]"
    if [ "${esp##*.}" = "esperado" ]; then
        if [ $rc -ne 0 ]; then falla "termino con error (codigo $rc)"; cat "$base.salida"
        elif diff <(tr -d '\r' < "$esp" | sed 's/[[:space:]][[:space:]]*/ /g; s/ $//') "$base.salida" >/dev/null; then echo "  OK"; ok=$((ok+1))
        else falla "la salida difiere (esperado < > obtenido)"; diff <(tr -d '\r' < "$esp" | sed 's/[[:space:]][[:space:]]*/ /g; s/ $//') "$base.salida"; fi
    else
        if [ $rc -ne 0 ] && grep -q -E "\[ERROR\].*($(tr -d '\r\n' < "$esp"))" "$base.salida"; then echo "  OK (aborta con error)"; ok=$((ok+1))
        else falla "deberia abortar con: $(cat "$esp")"; cat "$base.salida"; fi
    fi
done
echo; echo "Resultado: $ok OK, $mal con fallas"
[ $mal -eq 0 ]
