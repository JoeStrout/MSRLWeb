#!/bin/bash

# Generate API documentation from Raylib module files
# Outputs a markdown file with a table of all wrapped functions by module

OUTPUT_FILE="RAYLIB_API.md"

echo "# MSRLWeb Raylib API Reference" > "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "This document lists all Raylib functions available in MSRLWeb, organized by module." >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "Generated from: src/R*.cpp module files" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Extract functions from a module's source file
extract_functions() {
    local file_path=$1

    # Find all SetValue calls in the file
    awk '
        /raylibModule\.SetValue\("/ {
            match($0, /"([^"]+)"/)
            fname = substr($0, RSTART+1, RLENGTH-2)
            if (fname != "") print fname
        }
    ' "$file_path" | sort
}

# Extract functions for each module from their respective files
echo "Extracting rcore functions..."
RCORE_FUNCS=($(extract_functions "src/RCore.cpp"))

echo "Extracting rshapes functions..."
RSHAPES_FUNCS=($(extract_functions "src/RShapes.cpp"))

echo "Extracting rtextures functions..."
RTEXTURES_FUNCS=($(extract_functions "src/RTextures.cpp"))

echo "Extracting rtext functions..."
RTEXT_FUNCS=($(extract_functions "src/RText.cpp"))

echo "Extracting raudio functions..."
RAUDIO_FUNCS=($(extract_functions "src/RAudio.cpp"))

# Get the maximum count to know how many rows we need
MAX_COUNT=${#RCORE_FUNCS[@]}
[ ${#RSHAPES_FUNCS[@]} -gt $MAX_COUNT ] && MAX_COUNT=${#RSHAPES_FUNCS[@]}
[ ${#RTEXTURES_FUNCS[@]} -gt $MAX_COUNT ] && MAX_COUNT=${#RTEXTURES_FUNCS[@]}
[ ${#RTEXT_FUNCS[@]} -gt $MAX_COUNT ] && MAX_COUNT=${#RTEXT_FUNCS[@]}
[ ${#RAUDIO_FUNCS[@]} -gt $MAX_COUNT ] && MAX_COUNT=${#RAUDIO_FUNCS[@]}

# Write summary
echo "## Summary" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "- **rcore**: ${#RCORE_FUNCS[@]} functions" >> "$OUTPUT_FILE"
echo "- **rshapes**: ${#RSHAPES_FUNCS[@]} functions" >> "$OUTPUT_FILE"
echo "- **rtextures**: ${#RTEXTURES_FUNCS[@]} functions" >> "$OUTPUT_FILE"
echo "- **rtext**: ${#RTEXT_FUNCS[@]} functions" >> "$OUTPUT_FILE"
echo "- **raudio**: ${#RAUDIO_FUNCS[@]} functions" >> "$OUTPUT_FILE"
echo "- **Total**: $((${#RCORE_FUNCS[@]} + ${#RSHAPES_FUNCS[@]} + ${#RTEXTURES_FUNCS[@]} + ${#RTEXT_FUNCS[@]} + ${#RAUDIO_FUNCS[@]})) functions" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Write the table
echo "## Functions by Module" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| rcore | rshapes | rtextures | rtext | raudio |" >> "$OUTPUT_FILE"
echo "|-------|---------|-----------|-------|--------|" >> "$OUTPUT_FILE"

# Write each row
for ((i=0; i<$MAX_COUNT; i++)); do
    ROW="| "

    # rcore
    if [ $i -lt ${#RCORE_FUNCS[@]} ]; then
        ROW+="${RCORE_FUNCS[$i]}"
    fi
    ROW+=" | "

    # rshapes
    if [ $i -lt ${#RSHAPES_FUNCS[@]} ]; then
        ROW+="${RSHAPES_FUNCS[$i]}"
    fi
    ROW+=" | "

    # rtextures
    if [ $i -lt ${#RTEXTURES_FUNCS[@]} ]; then
        ROW+="${RTEXTURES_FUNCS[$i]}"
    fi
    ROW+=" | "

    # rtext
    if [ $i -lt ${#RTEXT_FUNCS[@]} ]; then
        ROW+="${RTEXT_FUNCS[$i]}"
    fi
    ROW+=" | "

    # raudio
    if [ $i -lt ${#RAUDIO_FUNCS[@]} ]; then
        ROW+="${RAUDIO_FUNCS[$i]}"
    fi
    ROW+=" |"

    echo "$ROW" >> "$OUTPUT_FILE"
done

echo "" >> "$OUTPUT_FILE"
echo "---" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "*Generated on $(date)*" >> "$OUTPUT_FILE"

echo ""
echo "Documentation generated: $OUTPUT_FILE"
echo ""
echo "Summary:"
echo "  rcore:      ${#RCORE_FUNCS[@]} functions"
echo "  rshapes:    ${#RSHAPES_FUNCS[@]} functions"
echo "  rtextures:  ${#RTEXTURES_FUNCS[@]} functions"
echo "  rtext:      ${#RTEXT_FUNCS[@]} functions"
echo "  raudio:     ${#RAUDIO_FUNCS[@]} functions"
echo "  ---"
echo "  Total:      $((${#RCORE_FUNCS[@]} + ${#RSHAPES_FUNCS[@]} + ${#RTEXTURES_FUNCS[@]} + ${#RTEXT_FUNCS[@]} + ${#RAUDIO_FUNCS[@]})) functions"
