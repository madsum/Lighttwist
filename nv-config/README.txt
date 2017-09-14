The nv-config folder contains configuration examples for an nvidia 
graphic card.

To use this configuration, copy the nvidia configuration settings using 
the following command:
cp nvidia-settings-rc ~/.nvidia-settings-rc

This configuration can then be loaded with the following command:
nvidia-settings –load-config-only

See the user guide (Lighttwist v1.0 : Installation de Lighttwist) for 
information on how to load this configuration at boot time.

To use an HD Optoma projector as display output, copy the xorg.conf file 
as follows:
sudo cp xorg.conf /etc/X11/

and reboot your computer.


