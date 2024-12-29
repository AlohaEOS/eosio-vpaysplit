#include <vpaysplit.hpp>

using namespace eosio;

void vpaysplit::setbuyer(name account, asset vote_eos, name vote_proxy, asset send_eos, uint32_t send_percent, name send_account, std::string send_memo) {
    require_auth(_self);
    buyer_index buyers(_self, _self.value);
    
    auto itr = buyers.find(account.value);
 
    if (itr == buyers.end()) {
        buyers.emplace(_self, [&](auto &row) {
            row.account = account;
            row.vote_eos = vote_eos;
            row.vote_proxy = vote_proxy;
            row.send_eos = send_eos;
            row.send_percent = send_percent;
            row.send_account = send_account;
            row.send_memo = send_memo;
        });
    } else {
        buyers.modify(itr, _self, [&](auto &row) {
            row.vote_eos = vote_eos;
            row.vote_proxy = vote_proxy;
            row.send_eos = send_eos;
            row.send_percent = send_percent;
            row.send_account = send_account;
            row.send_memo = send_memo;
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

void vpaysplit::transfer(name from, name to, asset quantity, std::string send_memo) {
    if (to != _self || from == _self) return;
    
    if ((from == "eosio.vpay"_n || from == "eosio.bpay"_n || (test_enabled && from == test_account))) {
        
        // Get total votes from producers table
        eosiosystem::producers_table producers("eosio"_n, "eosio"_n.value);
        auto prod = producers.get(_self.value, "must be a registered producer");
        check(prod.is_active, "must be an active producer");
        
        // Proxy tracking
        bool is_proxy_voting = false;
        
        // Calculate eos from vote weight
        double bp_vote_eos = weight2eos(prod.total_votes);
        
        // Iterate over buyers
        buyer_index buyers(_self, _self.value);
        asset send_eos = asset(0, eos_symbol);
        asset vote_eos;
        for (auto buyer = buyers.begin(); buyer != buyers.end(); buyer++) {
            vote_eos.set_amount(0);
            
            // Calculate vote eos
            // If buyer vote_eos is supplied use that. We ignore whether they are voting for us in this case.
            if (buyer->vote_eos.amount > 0) {
                vote_eos = buyer->vote_eos;
                
            // Otherwise use buyer vote_proxy and get the proxy details
            } else if (buyer->vote_proxy.to_string() != "") {
                eosiosystem::voters_table voters("eosio"_n, "eosio"_n.value);
                auto voter = voters.find(buyer->vote_proxy.value);
                
                // Make sure its a proxy, and its voting for us
                if (voter->is_proxy == 1) {
                    for (auto prod = voter->producers.begin(); prod != voter->producers.end(); prod++) {
                        if (prod->value == _self.value) {
                            is_proxy_voting = true;
                            vote_eos.set_amount(weight2eos(voter->last_vote_weight));
                        }
                    }
                }
            }
            
            // Calculate send eos
            // If static eos found, use that
            if (buyer->send_eos.amount > 0) {
                // If proxy set, require the vote
                if (buyer->vote_proxy.to_string() != "") {
                    if (is_proxy_voting) {
                        send_eos.set_amount(buyer->send_eos.amount);
                    }
                } else {
                    send_eos.set_amount(buyer->send_eos.amount);
                }
                
            // Otherwise if voting eos found, calculate from vote weight
            } else if (vote_eos.amount > 0) {
                send_eos.set_amount(vote_eos.amount / bp_vote_eos * ((double) buyer->send_percent / 100) * quantity.amount);
            }
            
            // Anything to send?
            if (send_eos.amount > 0) {
            
                // Where to send
                name send_account = buyer->account;
                if (buyer->send_account.to_string() != "") {
                    send_account = buyer->send_account;
                }
     
                // Send it
                action(
                    permission_level{_self, "active"_n},
                    "eosio.token"_n,
                    "transfer"_n,
                    std::make_tuple(_self, send_account, send_eos, buyer->send_memo)
                ).send(); 
            }           
        }
        
    }
}

double vpaysplit::weight2eos(double weight) {
    double time_diff = eosio::current_time_point().sec_since_epoch() - 946684800;
    return weight / pow(2, (time_diff / (86400 * 7) / 52));
}

