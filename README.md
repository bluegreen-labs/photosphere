# Photosphere

The photosphere project combines a Rico Theta S and a raspberry pi with a custom outdoor housing into a rugged 360 (outdoor) time-lapse camera. In my application I track changes in leaf development in a North-Eastern US forest. You can visit the project website at http://virtualforest.io. However, other applications are possible (hints on how to do nighttime images through the USB API are welcome).

The housing is made of standard PVC fittings, sitting on top of a garden fence post. The optics are covered by a glass lamp shade to provide optimal transmission and limited deformation (acrylic globes can be used as well). The only custom part is the plate on which the camera is mount using 1/4" steel thread. This plate is cut out of 3mm acrylic plastic. A vertical support also holds the raspberry pi to keep things tidy (see cad files). I use two 3mm plates extra rigidity (glued together). You can also use a 6mm top plate and a 3mm vertical plate. The inside of the "lens cover" is spray painted matte black to limit reflections.

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
<td>Ricoh Theta S</td>
<td>$350</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/pi.png"></td>
<td>Raspberry pi 2</td>
<td>$36</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/ubec.png"></td>
<td>Raspberry pi PoE hat</td>
<td>$25</td>
</tr>
<tr>
<td><img src="http://virtualforest.io/images/poe.png"></td>
<td>24V POE injector</td>
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
<td><img src="http://virtualforest.io/images/plexi.png"></td>
<td> 3mm Plexiglass sheet</td>
<td>$5</td>
</tr>

<tr>
<td><img src="http://virtualforest.io/images/rod.png"></td>
<td> 1/4" stainless steel rod</td>
<td>$1</td>
</tr>


<tr>
<td><img src="http://virtualforest.io/images/nut.png"></td>
<td> 1/4" (coupling) nuts</td>
<td>$1</td>
</tr>


<tr>
<td><img src="http://virtualforest.io/images/washer.png"></td>
<td> 1/4" washers</td>
<td>$1</td>
</tr>


<tr>
<td></td>
<td>TOTAL</td>
<td>~$550</td>
</tr>
</tbody>
</table>
