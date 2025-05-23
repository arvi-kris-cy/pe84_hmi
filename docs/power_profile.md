[Click here](../README.md) to view the README.

## Preliminary KPI: Power profile
The MCU power consumption is estimated by measuring the currents of the VBAT=3.3V and VDDD=1.8V domain. The VBAT and VDDD domain currents can be measured using the J25 and J26 jumpers respectively on the PSOC™ Edge Evaluation Kit.
Preliminary KPI data for power consumption is collected for ITSI and Sensory, during two stages of application operation:
1. No Active playback.
2. During active playback. 

**Table 1. CPU currents for mains powered configuration, VBAT=3.3V (J25), VDDD=1.8V (J26)**

Inferencing <br> Library | Power <br> Mode | CPU <br> Frequency | IBAT | IDDD | Total Average <br> Power <br> Consumption | I2S Playback  <br> State |
:-------- |:-------- | :----------  | :---------- | :-------- | :--------- | :---------
ITSI | High <br> Performance| 400 MHz | Floor: 21.8 mA <br> AVG: 22.8 mA | Floor: 3.3 mA <br> AVG: 3.4 mA |  AVG: 81.3 mW | Not active
ITSI | High <br> Performance| 400 MHz | Floor: 22 mA <br> AVG: 23.4 mA | Floor: 3.6 mA <br> AVG: 3.8 mA |  AVG: 84.0 mW | Active
Sensory | High <br> Performance| 400 MHz | Floor: 20.9 mA <br> AVG: 21.7 mA | Floor: 3.3 mA <br> AVG: 3.9 mA |  AVG: 78.6 mW | Not active
Sensory | High <br> Performance| 400 MHz | Floor: 21.2 mA <br> AVG: 22.4 mA | Floor: 3.6 mA <br> AVG: 4.3 mA |  AVG: 81.6 mW | Active