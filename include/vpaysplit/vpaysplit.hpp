#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio.system/eosio.system.hpp>
#include <math.h>

using namespace eosio;

const symbol eos_symbol = symbol("EOS", 4);
const name test_account = "waaaaaaaaaat"_n;
const bool test_enabled = false;

class [[eosio::contract("vpaysplit")]] vpaysplit : public contract {
    public:
        using contract::contract;
        
        [[eosio::action]]
        void setbuyer(name account, asset vote_eos, name vote_proxy, uint32_t percent, std::string memo);
        
        [[eosio::action]]
        void removebuyer(name account);
        
        [[eosio::action]]
        void resetbuyers();
    
        [[eosio::on_notify("eosio.token::transfer")]]
        void transfer(name from, name to, asset quantity, std::string memo);
        
    private:
        struct [[eosio::table]] buyer {
            name account;
            asset vote_eos;
            name vote_proxy;
            uint32_t percent;
            std::string memo;
            
            uint64_t primary_key() const { return account.value; }
        };
        
        typedef eosio::multi_index< "buyers"_n, buyer > buyer_index;
        
        double weight2eos(double weight);
};

