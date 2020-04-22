simple-wireless-tdma
====================

This is an ns3 (nsnam.org) module that originated with Hemanth Narra.
It has been updated to work under ns3 3.30.1.

Put this full tree in `<ns3>/src` and build using `./waf build` as usual.

## Example
I have been able to run the `tdma-example.cc` by placing it under the 
`scratch` directory and putting `tdmaSlots.txt` in the top-level directory so that 

`./waf --run tdma-example`

will run. 

or do 
`./waf --run "tdma-example --PrintHelp"`
  to see the possible arguments, including `--tdmaSlotsFile` to specify
  the relative path to a file specifying the TDMA slots. See sample
  under examples.

A configuration setting is missing somewhere to allow it to
run from the `src/simple-wireless-tdma/examples` directory.
