#!/bin/bash

# Simple script to run the web server and open the browser

# First, link the assets so what we're testing matches our source assets
rm -r build/assets
ln -s "../assets" build/assets

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "Stopping server..."
    kill $SERVER_PID 2>/dev/null
    exit 0
}

# Trap Ctrl+C and call cleanup
trap cleanup INT TERM

echo "Starting web server on http://localhost:8000"
echo ""

# Start the web server in the background
cd build
python3 -m http.server 8000 &
SERVER_PID=$!

# Wait a moment for the server to start
echo "Waiting for server to start..."
sleep 1.5

# Open in default browser (macOS)
echo "Opening msrlweb.html in your browser..."
open http://localhost:8000/msrlweb.html

echo ""
echo "Server running. Press Ctrl+C to stop"
echo ""

# Wait for the server process
wait $SERVER_PID

