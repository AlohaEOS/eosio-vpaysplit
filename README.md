# EOSIO Vote Pay Split
The `eosio-vpaysplit` contract is for splitting BP vote rewards to multiple parties at the time of claim. It is fully on-chain, transparent, flexible to various situations, and configurable via table data.

## Supported use cases when claiming BP rewards

- Pay an account based on percentage of vpay increase and dynamically calculated vote weight of a proxy, automatically detecting and stopping daily payment if proxy unregisters or unvotes the BP.

- Pay an account based on percentage of vpay increase and fixed vote weight.

## Building and deploying

### Dependencies

- eosio.cdt 1.7.0+ https://github.com/EOSIO/eosio.cdt/releases
- eosio.contracts 1.8.3+ https://github.com/EOSIO/eosio.contracts/releases

### Build

```
cd eosio-vpaysplit
EOSIO_CONTRACTS_PATH=/path/to/eosio.contracts ./build.sh
```

### Deploy

```
cd eosio-vpaysplit
cleos set contract BPACCOUNT ./build vpaysplit.wasm vpaysplit.abi -p BPACCOUNT
cleos set account permission alohaeosprod active --add-code
```

## Configuring

### Adding or updating payments

Example to setup payments for Colin Talks Crypto Rewards Proxy:

```
cleos push action BPACCOUNT setbuyer '{"account":"genereospool","vote_eos":"0.0000 EOS","vote_proxy":"colinrewards","percent":80,"memo":"BPACCOUNT"}' -p BPACCOUNT
```

Example to setup payments for Value Proxy:

```
cleos push action BPACCOUNT setbuyer '{"account":"voteforvalue","vote_eos":"0.0000 EOS","vote_proxy":"voteforvalue","percent":90,"memo":""}' -p BPACCOUNT
```

Example to setup payments for EOSDT Proxy:

```
cleos push action BPACCOUNT setbuyer '{"account":"eosdtgovernc","vote_eos":"0.0000 EOS","vote_proxy":"eosdtbpproxy","percent":85,"memo":"bp_deposit:BPACCOUNT"}' -p BPACCOUNT
```

Example to setup payments to "someaccount" without a specific proxy, but who votes with 10M EOS and requires 50% of the vpay increase:

```
cleos push action BPACCOUNT setbuyer '{"account":"someaccount","vote_eos":"10000000.0000 EOS","vote_proxy":"","percent":50,"memo":"hello there!"}' -p BPACCOUNT
```

### Removing payments

Example to remove payment to "someaccount":

```
cleos push action BPACCOUNT removebuyer '{"account":"someaccount"}' -p BPACCOUNT
```