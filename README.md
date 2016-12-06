12/6/16. Additions by Indiana University, Indiana Center for Biological Microscopy
Seth Winfree completed 2015.

This repo contains the IU-OpenSPIM additions made to support NEOS AOTF control 
and filterwheel control on the emission from within the plugin.  These are supported
in the multiview acquisition tab and via a new DAC slider control for setting AOTF
transmission and filterwheel position.



The [OpenSPIM](http://openspim.org/) project is a collaboration between multiple
labs to establish an accessible, easily adjustable light sheet microscope
system.

The *SPIMAcquisition* plugin for [Micro Manager](https://micro-manager.org) is
the primary means to acquire image stacks with the OpenSPIM setups.

# How to build

To build this plugin, two extra steps are required before letting Maven (or
any Integrated Development Environment with a Maven integration) build the
project: The Micro-Manager artifacts need to be installed locally because they
are not available via any public Maven repository yet.

Assuming that you already have a nightly build (e.g. by following the
[Micro-Manager-dev update site](http://sites.imagej.net/Micro-Manager-dev/)),
install the artifacts into your local Maven repository by executing the
following two commands inside the `ImageJ.app/` directory containing
Micro-Manager:

```bash
cd plugins/Micro-Manager
mvn install:install-file -DgroupId=org.micromanager -Dversion=1.4.21-SNAPSHOT \
	-Dpackaging=jar -DartifactId=MMJ_ -Dfile=MMJ_.jar
mvn install:install-file -DgroupId=org.micromanager -Dversion=1.4.21-SNAPSHOT \
	-Dpackaging=jar -DartifactId=MMCoreJ -Dfile=MMCoreJ.jar
```

After that, a simple `mvn -Dscijava.enforce.skip` will build the
`SPIMAcquisition` plugin which can then be installed by copying
`target/SPIMAcquisition-<version>.jar` into `Fiji.app/mmplugins/`.

# Installing into a Micro-Manager directory instead of Fiji

The SPIMAcquisition plugin depends on a couple of components that are shipped
with Fiji but not with Micro-Manager. Therefore, you will have to work a little
harder if you want to install the plugin into a vanilla Micro-Manager as
downloaded from [the Micro-Manager website](https://micro-manager.org):

1. Run `mvn dependency:copy-dependencies` in the SPIMAcquisition directory
2. From `target/dependency/` in that subdirectory, copy the `imglib2-*.jar` files to your Micro-Manager's `mmplugins/` subdirectory.
3. Copy `target/SPIMAcquisition-1.0.0-SNAPSHOT.jar` to the `mmplugins/` subdirectory, too.
4. Restart Micro-Manager.
