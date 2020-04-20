#include <vpaysplit.hpp>

using namespace eosio;

void vpaysplit::setbuyer(name account, asset vote_eos, uint32_t percent, std::string memo) {
    require_auth(_self);
    buyer_index buyers(_self, _self.value);
    
    auto itr = buyers.find(account.value);
 
    if (itr == buyers.end()) {
        buyers.emplace(_self, [&](auto &row) {
            row.account = account;
            row.vote_eos = vote_eos;
            row.percent = percent;
            row.memo = memo;
        });
    } else {
        buyers.modify(itr, _self, [&](auto &row) {
            row.vote_eos = vote_eos;
            row.percent = percent;
            row.memo = memo;
        });
    }   
}

void vpaysplit::removebuyer(name account) {
    require_auth(_self);
    buyer_index buyers(_self, _self.value);
    
    auto itr = buyers.find(account.value);
    
    check(itr != buyers.end(), "buyer not found");
    buyers.erase(itr);
}

void vpaysplit::transfer(name from, name to, asset quantity, std::string memo) {
    if (to != _self || from == _self) return;
    
    if ((from == "eosio.vpay"_n || (test_enabled && from == test_account)) && to == _self) {
        
        // Get total votes from producers table
        eosiosystem::producers_table producers("eosio"_n, "eosio"_n.value);
        auto prod = producers.get(_self.value, "must be a registered producer");
        check(prod.is_active, "must be an active producer");
        
        // Calculate eos from vote weight
        double time_diff = eosio::current_time_point().sec_since_epoch() - 946684800;
        double weight = pow(2, (time_diff / (86400 * 7) / 52));
        double bp_vote_eos = prod.total_votes / weight;
        
        // Iterate over buyers
        buyer_index buyers(_self, _self.value);
        asset send_eos = asset(0, eos_symbol);
        for (auto buyer = buyers.begin(); buyer != buyers.end(); buyer++) {
        
            // Calculate send amount
            send_eos.set_amount(buyer->vote_eos.amount / bp_vote_eos * ((double) buyer->percent / 100) * quantity.amount);
            
            // And send it
            if (send_eos.amount > 0) {
                action(
                    permission_level{_self, "active"_n},
                    "eosio.token"_n,
                    "transfer"_n,
                    std::make_tuple(_self, buyer->account, send_eos, buyer->memo)
                ).send(); 
            }           
        }
        
    }
}

