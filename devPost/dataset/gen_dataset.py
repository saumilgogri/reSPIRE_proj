import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# Exponential function
def exp_rise(time=10, max=40, shift=0, exp=0.2):
    mult=max-shift
    expt=[]
    t_arr = np.linspace(start=0, stop=time, num=time*10)
    expt_max = mult*np.exp(time*exp)/mult
    for t in t_arr:
        expt.append(shift+((mult*np.exp(t*exp))/expt_max))
    return expt

# Saturating Exponential function 
def exp_sat_rise(time=10, mult=20, shift = -20, exp=2):
    exp_sat =[]
    t_arr = np.linspace(start=0, stop=time, num=time*10)
    for t in t_arr:
            exp_sat.append(shift + mult*(1 - np.exp(-t/exp)))
    return exp_sat

def exp_sat_fall(time=10, mult=40, exp=5):
    exp_sat =[]
    t_arr = np.linspace(start=0, stop=time, num=time*10)
    for t in t_arr:
        exp_sat.append(mult*(np.exp(-t/exp)))
    return exp_sat

# Ramp Increment function
def ramp(time=10, left=5, right=40):
    ramp = []
    slope = (right-left)/time
    t_arr = np.linspace(start=0, stop=time, num=time*10)
    for t in t_arr:
        ramp.append(left + (t*slope))
    return ramp

# Square Function
def square (time=10,val=40):
    sqr = []
    t_arr = np.linspace(start=0, stop=time, num=time*10)
    for t in t_arr:
        sqr.append(val)
    return sqr

# Noise Function
def add_gauss_noise(arr,amp=0.5):
    noise = amp*np.random.normal(size=len(arr))
    arr = arr + noise
    return arr

def add_time(time):
    return np.linspace(start=0, stop=time, num=time*10)
    


##### VOLUME MODE #####
time=[]
pressure=[]
flow=[]
volume=[]
tot_time=0

##2sec
tot_time+=2
pressure.extend(square(2,0))
flow.extend(square(2,0))
volume.extend(square(2,0))

for j in range (200):
    ##15sec
    tot_time+=15
    pressure.extend(ramp(15,5,40))
    flow.extend(square(15,15))
    volume.extend(ramp(15,0,20))
    
    ##2sec
    tot_time+=2
    pressure.extend(square(2,35))
    flow.extend(square(2,0))
    volume.extend(square(2,20))
    
    ##25sec
    tot_time+=25
    pressure.extend(exp_sat_fall(25,35))
    flow.extend(exp_sat_rise(25,15,-15))
    volume.extend(exp_sat_fall(25,20))

##total_time=86sec
time.extend(add_time(tot_time))


pressure = add_gauss_noise(pressure)
flow = add_gauss_noise(flow)
volume = add_gauss_noise(volume)


vol_mod_fig, (pre, flo, vol) = plt.subplots(3)
vol_mod_fig.suptitle('Volume Mode')
pre.plot(time,pressure)
flo.plot(time,flow)
vol.plot(time,volume)

vol_mode_df = pd.DataFrame(
    {'Time': time,
     'Pressure': pressure,
     'Flow': flow,
     'Volume': volume
    })

vol_mode_df.to_csv(r'/Users/saumilgogri/reSPIRE_proj/devPost/dataset/Volume_Mode.csv')

##### PRESSURE MODE #####


time=[]
pressure=[]
flow=[]
volume=[]
tot_time=0

##2sec
tot_time+=2
pressure.extend(square(2,0))
flow.extend(square(2,0))
volume.extend(square(2,0))

for j in range (200):
    ##15sec
    tot_time+=15
    pressure.extend(square(15,40))
    flow.extend(ramp(10,15,0))
    flow.extend(square(5,0))
    volume.extend(exp_rise(10,20))
    volume.extend(square(5,20))
      
    ##25sec
    tot_time+=25
    pressure.extend(square(25,0))
    flow.extend(exp_sat_rise(15,15,-15,3))
    flow.extend(square(10,0))
    volume.extend(exp_sat_fall(25,20))

##total_time=86sec
time.extend(add_time(tot_time))


pressure = add_gauss_noise(pressure)
flow = add_gauss_noise(flow)
volume = add_gauss_noise(volume)

pres_mode_df = pd.DataFrame(
    {'Time': time,
     'Pressure': pressure,
     'Flow': flow,
     'Volume': volume
    })
pres_mode_df.to_csv(r'/Users/saumilgogri/reSPIRE_proj/devPost/dataset/Pressure_Mode.csv')


pre_mode_fig, (pre, flo, vol) = plt.subplots(3)
pre_mode_fig.suptitle('Pressure Mode')
pre.plot(time,pressure)
flo.plot(time,flow)
vol.plot(time,volume)