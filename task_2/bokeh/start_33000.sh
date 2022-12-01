#!/bin/sh
bokeh serve --address 195.19.32.95 --port 33000 --allow-websocket-origin=195.19.32.95:33000 --show $1
