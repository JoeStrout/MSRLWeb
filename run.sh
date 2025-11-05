#!/bin/bash

# Simple script to run the web server and open the browser

echo "Starting web server on http://localhost:8000"
echo "Opening game.html in your browser..."
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

# Open in default browser (macOS)
open http://localhost:8000/msrlweb.html

# Start the web server in the build folder
cd build
python3 -m http.server 8000

