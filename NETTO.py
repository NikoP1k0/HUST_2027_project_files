# -*- coding: utf-8 -*-
"""
Created on Tue Dec 30 20:39:03 2025

@author: oscar
"""

from SOLPLAN import *
from FORB import *

"""
Följande gör sammanställningar mellan produktion och konsumption
"""

def sectional_net(sections, speed, solar_car):
    prod_list = sectional_production(sections, speed, solar_car)
    cons_list = sectional_consumption(sections, speed, solar_car)
    tot_list = []
    for i in range(len(prod_list)):
        prod = prod_list[i]
        cons = cons_list[i]
        net = prod[0] - cons[0]
        tot_list.append([prod[1], prod[0], cons[0], net])
    return tot_list

def even_net(distance_list, num_secs, speed, solar_car):
    prod_list = even_production(distance_list, speed, num_secs, solar_car)
    cons_list = even_consumption(distance_list, speed, num_secs, solar_car)
    tot_list = []
    #print(len(cons_list))
    for i in range(len(prod_list)):
        prod = prod_list[i]
        cons = cons_list[i]
        net = prod[0] - cons[0]
        tot_list.append([prod[1], prod[0], cons[0], net])
    return tot_list

def plot_sectional_net(sections, speed, solar_car):
    sec_net = sectional_net(sections, speed, solar_car)
    X = np.array(sec_net).transpose()
    A = X[0]
    B = X[1]
    C = X[2]
    D = X[3]
    plt.plot(A, B, "b-")
    plt.plot(A, C, "r-")
    plt.plot(A, D, "g-")
    plt.show()
    return X

def plot_even_net(all_distances, num_secs, speed, solar_car):
    sec_net = even_net(all_distances, num_secs, speed, solar_car)
    X = np.array(sec_net).transpose()
    A = X[0]
    B = X[1]
    C = X[2]
    D = X[3]
    plt.plot(A, B, "b-")
    plt.plot(A, C, "r-")
    plt.plot(A, D, "g-")
    plt.show()
    return X
