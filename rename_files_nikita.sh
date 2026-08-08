#!/bin/bash
# Rename completo Lotei -> Nikita: arquivos, classes, .pro, .qrc e
# os caminhos no cartao SD.
#
# Rodar de dentro de nikita-qflipper/
#     bash rename_files_nikita.sh
#
# ATENCAO: copie /ext/lotei/ para /ext/nikita/ no cartao ANTES de rodar,
# senao a memoria gravada (facts + proven moves) fica orfa.

set -e

APP="application"
[ -f "$APP/loteibackend.cpp" ] || { echo "rode de dentro de nikita-qflipper/"; exit 1; }

echo "==> backup"
STAMP=$(date +%Y%m%d_%H%M%S)
cp -R "$APP" "/tmp/application_pre_rename_$STAMP"
echo "    /tmp/application_pre_rename_$STAMP"

# ---------------------------------------------------------------------
# 1. Renomear os arquivos
# ---------------------------------------------------------------------
echo "==> 1/6 renomeando arquivos"
mv "$APP/loteibackend.cpp"            "$APP/nikitabackend.cpp"
mv "$APP/loteibackend.h"              "$APP/nikitabackend.h"
mv "$APP/components/LoteiChat.qml"    "$APP/components/NikitaTalk.qml"

# ---------------------------------------------------------------------
# 2. Simbolos C++: classes e include
# ---------------------------------------------------------------------
echo "==> 2/6 classes e includes"
for f in "$APP/nikitabackend.cpp" "$APP/nikitabackend.h" \
         "$APP/application.cpp"   "$APP/application.h"; do
    sed -i '' \
        -e 's/LoteiBackend/NikitaBackend/g' \
        -e 's/LoteiPalette/NikitaPalette/g' \
        -e 's/loteibackend\.h/nikitabackend.h/g' \
        -e 's/loteibackend\.cpp/nikitabackend.cpp/g' \
        -e 's/m_lotei\b/m_nikita/g' \
        "$f"
done

# ---------------------------------------------------------------------
# 3. Build: .pro e .qrc
# ---------------------------------------------------------------------
echo "==> 3/6 application.pro e qml.qrc"
sed -i '' \
    -e 's/loteibackend\.cpp/nikitabackend.cpp/g' \
    -e 's/loteibackend\.h/nikitabackend.h/g' \
    -e 's/loteibackend\.\*/nikitabackend.*/g' \
    "$APP/application.pro"

sed -i '' 's#components/LoteiChat\.qml#components/NikitaTalk.qml#g' "$APP/qml.qrc"

# ---------------------------------------------------------------------
# 4. QML: uso do componente
# ---------------------------------------------------------------------
echo "==> 4/6 referencias ao componente no QML"
find "$APP" -name '*.qml' -print0 | xargs -0 sed -i '' 's/\bLoteiChat\b/NikitaTalk/g'

# ---------------------------------------------------------------------
# 5. Caminhos no cartao SD
# ---------------------------------------------------------------------
echo "==> 5/6 /ext/lotei -> /ext/nikita"
sed -i '' 's#/ext/lotei#/ext/nikita#g' "$APP/nikitabackend.cpp"

# ---------------------------------------------------------------------
# 6. Sobras: comentarios, logs e qualquer "lotei" solto
# ---------------------------------------------------------------------
echo "==> 6/6 comentarios e strings restantes"
sed -i '' \
    -e 's/\bLotei\b/Nikita/g' \
    -e 's/\blotei\b/nikita/g' \
    "$APP/nikitabackend.cpp" "$APP/nikitabackend.h"

# =====================================================================
echo ""
echo "======================= VERIFICACAO ========================="

echo ""
echo "-- arquivos no lugar:"
ls -1 "$APP"/nikitabackend.* "$APP/components/NikitaTalk.qml" 2>/dev/null || echo "   !! faltando"

echo ""
echo "-- sobrou 'lotei' em qualquer forma? (esperado: vazio)"
grep -rni "lotei" "$APP" 2>/dev/null || echo "   ok, limpo"

echo ""
echo "-- .pro aponta pro arquivo novo:"
grep -n "nikitabackend" "$APP/application.pro" || echo "   !! NAO ENCONTRADO"

echo ""
echo "-- .qrc aponta pro QML novo:"
grep -n "NikitaTalk" "$APP/qml.qrc" || echo "   !! NAO ENCONTRADO"

echo ""
echo "-- classes renomeadas:"
grep -n "class NikitaBackend\|class NikitaPalette" "$APP/nikitabackend.h" || echo "   !! NAO ENCONTRADO"

echo ""
echo "-- membros em application.h:"
grep -n "NikitaBackend m_nikita\|NikitaPalette m_palette" "$APP/application.h" || echo "   !! NAO ENCONTRADO"

echo ""
echo "============================================================"
echo "Build limpo e obrigatorio (o Makefile antigo cita os arquivos velhos):"
echo "  rm -rf build_mac && ./build_mac.sh"
echo ""