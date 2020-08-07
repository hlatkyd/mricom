##Testing setup

By default the subprograms blockstim, ttlctrl, test-console use different
IO pins of the NI device. See conf/settings for specifics.


# Testing stimulation paradigm

Launch a block stimulation design named 'test' defined in conf/blockstim.conf.
A digital output should send the appropriate series of TTL signals.
Check results with a scope. No other IO pins needed.
```
./bin/blockstim design test
```

# Analog acquisition



#Simulate consequtive sequences

This tests all the communication routes and is intended to simulate a full fMRI
experiment.
Use 2 IO pins to simulate the console.
```
./test/test_run
```


