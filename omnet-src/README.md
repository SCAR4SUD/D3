# Scar4Sud OMNeT++ Simulation

This is an OMNeT++ project that simulates secure communication between components in a car network.

## Setup

This projects need the following dependencies:

- openssl (version >= 1.0)
- softhsm2

To compile the simulation run in `Progetto/src`:

```bash
$ opp_makemake -f --deep -lssl -lcrypto -lsofthsm2
$ make clean
$ make
```

To initialize the SoftHSMv2 library for use with this project drop the contents of `hsm_object_store` into the where softhsm tokens are stored. By default it can be found in: `/var/lib/softhsm/tokens/[token_id]/`.

## Structure

### src

Contains the simulation source files:

-  The NED language defined network modules and their c++ class implementations. 
- Collection of c++ functions for simulating ECU and TPM crypto behavior.
- Collection of c++ classes, and function for simulating HSM behaviour.

### storage

Contains folder representing all nodes in the network that have for the sake of the simulation a storage. The ECU "storages" thus follow the id of the various lords. 
