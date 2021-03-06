# Photosphere

The photosphere project combines a Rico Theta Z1 and a raspberry pi with a custom outdoor housing into a rugged 360 (outdoor) time-lapse camera. In my application I track changes in leaf development in a North-Eastern US forest. You can visit the project website at http://virtualforest.io.

The housing is made of standard PVC fittings, sitting on top of a garden fence post. The optics are covered by a glass lamp shade to provide optimal transmission and limited deformation (acrylic globes can be used as well). The only custom part is the mounting setup of the camera which is a 3D printed PLA setup. Black PLA is used to limit internal reflections.

Constructed and placed in the forest the camera looks like a garden lamp (see figure). Here the blue wire is the ethernet cable which runs to a nearby hub, and serves as the internet and power connection. The yellow wire is the ground wire for the surge protector, to protect the camera from voltage spikes due to nearby lightning strikes.

<img src="http://virtualforest.io/images/camera.jpg">

## Installation

To install the software clone the project onto a raspberry pi and run the install script.

```bash
git clone https://github.com/khufkens/photosphere.git
sh install_photosphere.sh
```
You will have to edit the crontab manually to add the server argument (for now).

## Parts List

I assume that common items such as screws and glue (pvc, silicone) are available to makers.

<table style="width:80%">
<tbody>
<tr>
<td></td>
<td><b>Item</b></td>
<td><b>Price ($)</b></td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/thetas.png"></td>
<td>Ricoh Theta 360 camera</td>
<td>~$300+</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/pi.png"></td>
<td>Raspberry pi 3B+</td>
<td>$36</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/pihat.jpg"></td>
<td>Raspberry pi PoE hat</td>
<td>$25</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/poe.png"></td>
<td>48V POE injector</td>
<td>$10</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/surge.png"></td>
<td>APC surge protector</td>
<td>$18</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/cable.png"></td>
<td>200ft / 50m Cat5e cable</td>
<td>$13</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/globe.png"></td>
<td>8" glass globe lamp cover</td>
<td>$35</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/pipe.png"></td>
<td>4" (10cm) PVC pipe</td>
<td>$20</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/coupler.png"></td>
<td>4" (10cm) PVC coupler</td>
<td>$2.5</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/pole.png"></td>
<td>3.5" Wooden Fence post</td>
<td>$5</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/spike.png"></td>
<td>36" ground anchor</td>
<td>$30</td>
</tr>

<tr>
<td></td>
<td>TOTAL</td>
<td>~$500+</td>
</tr>
</tbody>
</table>
