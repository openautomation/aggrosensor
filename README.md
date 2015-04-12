aggrosensor
===========

Sensor configuration and aggregation code.

-----------

####Measurement Example Output:
{"measurement":{"label":"upper temp","datum":23.53}}

####Example Complete Sensor Entry:
{"index":0,"label":"upper temp","sensorID":"ENV-TMP","msMeasurementPeriod":5000,"pins":[2,14]}

####Commands:
- {entries}
- {getEntry:\<index\>}
- {removeEntry:\<index\>}
- {setEntry:{\<one or more sensor entry key-value pairs\>}}

####Temporary Commands for writing to pins
- {writeD:{pin:14, value:1}}
- {writeA:{pin:14, value:1}}
