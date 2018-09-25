#!/usr/bin/python3

import numpy as np
import os
import matplotlib.pyplot as plt
#from __future__ import print_function

from math import cos, sin, acos, asin, atan2, sqrt, pi, radians, degrees
def target_phi2(z, z0, Rc, Rr, theta, beta):
    a = cos(beta)*cos(theta)
    b = sin(beta)
    c = cos(beta)*sin(theta)/Rc
    d = (-cos(beta)*sin(theta)*z0 + Rr*sin(beta))/Rc
    gamma = sqrt(a*a + b*b)
    delta = atan2(b,a)
    return asin((c*z+d)/gamma) - delta



# strip creation
def make_targets2(low, up, w, p):
    strip_center = np.concatenate( (np.arange(-p,low,-p),np.arange(0,up,p)) )
    strip_center = np.sort(strip_center)
    strip_min = np.array([s-0.5*w for s in strip_center])
    strip_max = np.array([s+0.5*w for s in strip_center])
    strips = np.reshape(np.concatenate((strip_min,strip_center,strip_max),axis=0), (3, len(strip_min)) )
    return strips.T


if __name__ == '__main__':

    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    # TPC settings
    length = 2304.0
    zmin = -(length*0.5) # mm
    zmax = zmin * -1.0
    print( 'TPC length: %1.0f mm' % (zmax - zmin) )

    cathode_radius = 109.25 # mm
    print( 'Cathode Radius =', cathode_radius, 'mm' )
    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    # Al targets / STRIPS settings
    pitch = 265. # mm :  9 strips
    strip_z = 6.0 # mm
    delta_z = pitch - strip_z
    print( '\ttarget width %1.0f mm   pitch %1.0f mm' % (strip_z, pitch) )
    #######################################################################
    # strip creation
    limits = make_targets2(zmin, zmax, strip_z, pitch)
    print( "TARGETS [cm]" )
    print( "\n".join( str(targ*0.1) for targ in limits ) )
    off_z = limits[0][0] - zmin
    print( '\tz offset %1.0f mm' % off_z )
    stripz = limits[:,1]
    #######################################################################


    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
    # RUN settings  
    Npoints = int(zmax-zmin) + 1
    z = np.linspace(zmin, zmax, Npoints)
    print( 'Number of z Points %d' % Npoints )
    #print( "\n".join( ('%1.3f' % zz) for zz in z ) )
    phi2 = np.vectorize(target_phi2)
    r0 = (cathode_radius+0.005)*0.1 # cm
    print( 'e- starting radius ', r0, 'cm' )
    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    # LASER settings
    off_phi_deg = 25. # deg
    off_phi = radians(off_phi_deg)
    print( '\tphi offset %1.0f deg' % off_phi_deg  )

    z0=-1172.5 # mm
    print( 'laser injection point z0 =', z0, 'mm' )

    # conf 1
    beam_inclination1 = 3.0 # deg
    print( 'Beam Inclination (y) = %1.1f deg = %1.3f rad' % (beam_inclination1, radians(beam_inclination1)) )
    plane_inclination1 = -50.0 #deg
    print( 'Plane Inclination (z) = %1.1f deg = %1.3f rad' % (plane_inclination1, radians(plane_inclination1)) )

    # conf 2
    beam_inclination2 = 2.0 # deg
    print( 'Beam Inclination (y) = %1.1f deg = %1.3f rad' % (beam_inclination2, radians(beam_inclination2)) )
    plane_inclination2 = -40.0 #deg
    print( 'Plane Inclination (z) = %1.1f deg = %1.3f rad' % (plane_inclination2, radians(plane_inclination2)) )

    rod_pos = 27.35 # mm
    Rr = cathode_radius + rod_pos
    phimax = acos(cathode_radius/Rr) + off_phi
    anglemax = np.empty(Npoints)
    anglemax.fill(degrees(phimax))
    print( 'horizon %1.3f deg' % degrees(phimax) )
    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    print(stripz)
    print(np.degrees(phi2(stripz,z0,cathode_radius,Rr,radians(beam_inclination1),radians(plane_inclination1)) + off_phi ))

    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    # LASER PROFILE
    laser_prof1 = np.degrees(phi2(z,z0,cathode_radius,Rr,radians(beam_inclination1),radians(plane_inclination1)) + off_phi )

    laser_prof2 = np.degrees(phi2(z,z0,cathode_radius,Rr,radians(beam_inclination2),radians(plane_inclination2)) + off_phi )

    #%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    # PLOT


    #fig = plt.figure(num=None, figsize=(18, 16), dpi=200, facecolor='w', edgecolor='k')
    plotname = 'Targets Position - %1.2fcm pitch - and Laser z-phi Profile' % (pitch*0.1)
    plt.title(plotname)
    plt.plot(z,laser_prof1,'b-',label='3deg y, -50deg z')
    plt.plot(z,laser_prof2,'r-',label='2deg y, -40deg z')
    plt.plot(z,anglemax,'k--',label='horizon')
    plt.xticks(np.arange(-1200,1200,200))
    plt.yticks(np.arange(0.,degrees(phimax)+3.,5.))
    plt.xlabel('z [mm]')
    plt.ylabel('phi [degrees]')
    
    ymin, ymax = plt.ylim()
    for a,xxx,b in limits:
        plt.axvspan(a, b, ymin, ymax, facecolor='g', alpha=0.4)
    plt.plot([],[], 'g',alpha=0.4,label='Al strips') # hack to show only one entry in the legend

    plt.axvline(zmin, ymin, ymax, color='k')
    plt.axvline(zmax, ymin, ymax, color='k')

    fig=plt.gcf()
    fig.set_size_inches(18, 16)
    figname = 'Targets_Position-%1.2fcm_pitch-Laser_z-phi_Profile' % (pitch*0.1)
    fig.canvas.set_window_title(figname)
    fig.subplots_adjust(left=0.04,bottom=0.05,right=0.99,top=0.96)

    plt.legend(loc='best')
    plt.grid()
    plt.show()

    fig.savefig('final_target_position_with_laser_profile.png')
