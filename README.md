## Datapoint types

In the table below you can find how to define your datapoints:

|name|size|converter|return type|remarks|
|---|---|---|---|---|
|Temperature|2|div10|float||
|Temperature short|1|noconv|uint8_t|equivalent to Mode|
|Status|1|noconv|bool|this is the same as 'Temperature short' and 'Mode'. The  `uint8_t` value will be implicitely converted to bool.|
|Hours|4|div3600|float||
|Count|4|noconv|uint32_t||
|Count short|2|noconv|uint16_t||
|Mode|1|noconv|uint8_t|possibly castable to ENUM|
|CoP|1|div10|float||

When reading or writing you have to use the exact datatype. Implicit casting of datatypes will not work or will give unexpected/undefined results.
