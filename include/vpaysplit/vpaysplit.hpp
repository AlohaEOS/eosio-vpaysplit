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
        
        /**
         * Set a buyer
         * 
         * @param account Buyer account
         * @param vote_eos Buyer voting EOS if static
         * @param vote_proxy Buyer voting proxy (if vote_eos == 0)
         * @param send_eos Static EOS to send
         * @param send_percent Percent of voting eos to send (if send_eos == 0)
         * @param send_account Account to send to
         * @param send_memo Memo when sending
         */
        [[eosio::action]]
        void setbuyer(name account, asset vote_eos, name vote_proxy, asset send_eos, uint32_t send_percent, name send_account, std::string send_memo);
        
        /**
         * Remove a buyer
         * 
         * @param account Buyer account
         */
        [[eosio::action]]
        void removebuyer(name account);
        
        /**
         * Reset all buyers
         */
        [[eosio::action]]
        void resetbuyers();
    
        /**
         * Transfer handler
         * 
         * @param from Sender
         * @param to Receiver
         * @param quantity Quantity
         * @param send_memo Memo
         */
        [[eosio::on_notify("eosio.token::transfer")]]
        void transfer(name from, name to, asset quantity, std::string send_memo);
        
    private:
        struct [[eosio::table]] buyer {
            name account;           // Buyer account 
            asset vote_eos;         // Buyer voting EOS if static
            name vote_proxy;        // Buyer voting proxy (if vote_eos == 0)
            asset send_eos;         // Static EOS to send
            uint32_t send_percent;  // Percent of voting eos to send (if send_eos == 0)
            name send_account;      // Account to send to
            std::string send_memo;  // Memo to use when sending
            
            uint64_t primary_key() const { return account.value; }
        };
        
        typedef eosio::multi_index< "buyers"_n, buyer > buyer_index;
        
        double weight2eos(double weight);
};

