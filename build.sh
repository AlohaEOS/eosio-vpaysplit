#!/bin/sh

if [ -z "$EOSIO_CONTRACTS_PATH" ]; then
    EOSIO_CONTRACTS_PATH="$HOME/export/eosio.contracts"
fi
    
eosio-cpp -abigen -I include/vpaysplit -I $EOSIO_CONTRACTS_PATH/contracts/eosio.system/include -o build/vpaysplit.wasm src/vpaysplit.cpp