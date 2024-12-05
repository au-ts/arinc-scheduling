Proof of Concept for ARINC scheduler in Microkit, using the three domain example.

Operation of example:
- Partition 3 prints the values received.
    - Prints the message from Partition 2 if number is even (single send, single receive (SS/SR) port
      messages count up)
    - Prints the broadcast message from Partition 1 if odd (single send, multiple receive (SS/MR) port
      messages count down from 1000)
    - When message from SS/SR == SS/MR (i.e. 500), print different message

Notes:
- Only periodic processes
- No overrun handling
- No buffering time to account for scheduler processing in between partitions
- Hard coded configuration of partitions (sdfgen related work will address this once it is ready)
- sPD code different for each partition currently. Likely can be generalised when above work completed
  (so one .c file can be used for all partitions)
- Currently has VALID/INVALID and FULL/EMPTY flags for ports: at the moment functionally they do not
  do anything different, but the structure is there for future extensions (e.g. valid message depends
  on age of message)
