#include <vpaysplit.hpp>

using namespace eosio;

void vpaysplit::setbuyer(name account, asset vote_eos, name vote_proxy, uint32_t percent, std::string memo) {
    require_auth(_self);
    buyer_index buyers(_self, _self.value);
    
    auto itr = buyers.find(account.value);
 
    if (itr == buyers.end()) {
        buyers.emplace(_self, [&](auto &row) {
            row.account = account;
            row.vote_eos = vote_eos;
            row.vote_proxy = vote_proxy;
            row.percent = percent;
            row.memo = memo;
        });
    } else {
        buyers.modify(itr, _self, [&](auto &row) {
            row.vote_eos = vote_eos;
            row.vote_proxy = vote_proxy;
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

void vpaysplit::resetbuyers() {
    require_auth(_self);
    buyer_index buyers(_self, _self.value);
    for(auto itr = buyers.begin(); itr != buyers.end();) {
        itr = buyers.erase(itr);
    }
}

void vpaysplit::transfer(name from, name to, asset quantity, std::string memo) {
    if (to != _self || from == _self) return;
    
    if ((from == "eosio.vpay"_n || (test_enabled && from == test_account))) {
        
        // Get total votes from producers table
        eosiosystem::producers_table producers("eosio"_n, "eosio"_n.value);
        auto prod = producers.get(_self.value, "must be a registered producer");
        check(prod.is_active, "must be an active producer");
        
        // Calculate eos from vote weight
        double bp_vote_eos = weight2eos(prod.total_votes);
        
        // Iterate over buyers
        buyer_index buyers(_self, _self.value);
        asset send_eos = asset(0, eos_symbol);
        asset vote_eos;
        for (auto buyer = buyers.begin(); buyer != buyers.end(); buyer++) {
            vote_eos.set_amount(0);
            
            // If buyer vote_eos is supplied use that
            if (buyer->vote_eos.amount > 0) {
                vote_eos = buyer->vote_eos;
                
            // Otherwise use buyer vote_proxy and get the proxy details
            } else if (buyer->vote_proxy.to_string() != "") {
                eosiosystem::voters_table voters("eosio"_n, "eosio"_n.value);
                auto voter = voters.get(buyer->vote_proxy.value);
                
                // Make sure its a proxy that is voting for us
                if (voter.is_proxy == 1) {
                    for (auto prod = voter.producers.begin(); prod != voter.producers.end(); prod++) {
                        if (prod->value == _self.value) {
                            vote_eos.set_amount(weight2eos(voter.last_vote_weight));
                        }
                    }
                }
                
            }
            
            // Make sure some voting eos was found
            if (vote_eos.amount > 0) {
            
                // Calculate send amount
                send_eos.set_amount(vote_eos.amount / bp_vote_eos * ((double) buyer->percent / 100) * quantity.amount);
                
                // Send it
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
}

double vpaysplit::weight2eos(double weight) {
    double time_diff = eosio::current_time_point().sec_since_epoch() - 946684800;
    return weight / pow(2, (time_diff / (86400 * 7) / 52));
}

