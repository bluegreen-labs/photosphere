# Photosphere

The photosphere project combines a Rico Theta 360 camera and a raspberry pi with a custom outdoor housing into a rugged 360 (outdoor) time-lapse camera. In my application I track changes in leaf development in a Belgian forest. You can visit the project website at http://virtualforest.io.

The housing is made of standard PVC fittings, sitting on top of a garden fence post. The optics are covered by a glass lamp shade to provide optimal transmission and limited deformation (acrylic globes can be used as well). The only custom part is the mounting setup of the camera which is a 3D printed PLA setup. Black PLA is used to limit internal reflections.

Constructed and placed in the forest the camera looks like a garden lamp (see figure). An ethernet cable, which runs to a nearby hub, serves as the internet and power connection. The setup has a ground wire for surge protection due to voltage spikes from any nearby lightning strikes.

<img src="http://virtualforest.io/img/featured.jpg">

## Installation

To install the software clone the project onto a raspberry pi and run the install script.

```bash
git clone https://github.com/bluegreen-labs/photosphere.git
sh install_photosphere.sh
```
You will have to edit the crontab manually. Data is uploaded to github for viewing, but additional lines can be added to upload captures elsewhere for permanent storage (either on- or offline).

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
<td>Ricoh Theta 360 camera</td>
<td>~$300+</td>
</tr>
<tr>
<td>Raspberry pi 3B+</td>
<td>$36</td>
</tr>
<tr>
<td>Raspberry pi PoE hat</td>
<td>$25</td>
</tr>
<tr>
<td>48V POE injector</td>
<td>$10</td>
</tr>
<tr>
<td>APC surge protector</td>
<td>$18</td>
</tr>
<tr>
<td>200ft / 50m Cat5e cable</td>
<td>$13</td>
</tr>
<tr>
<td>8" glass globe lamp cover</td>
<td>$35</td>
</tr>
<tr>
<td>4" (10cm) PVC pipe</td>
<td>$20</td>
</tr>
<tr>
<td>4" (10cm) PVC coupler</td>
<td>$2.5</td>
</tr>
<tr>
<td>3.5" Wooden Fence post</td>
<td>$5</td>
</tr>
<tr>
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
