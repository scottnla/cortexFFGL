Title: cortexFFGL
Author: Nathan S Lachenmyer
Date Started: August 2013
Last Modified: September 2013

=CORTEX FFGL=
Cortex creates graphics based off of Kuver's form constants.  The original paper documenting the mathematics can be found at the following URL:

www.math.utah.edu/~bresslof/publications/01-1.pdf

This is an FFGL port of the original standalone program (located at http://www.github.com/scottnla/cortex) that exposes the shader's uniforms as sliders and buttons in FFGL renderers such as FFRend and Resolume.  This makes it MUCH easier to control parameters, beat sync them, and blend form constants.

=PARAMETERS=

The parameters are the same as in the original cortex.  There are eight primary geometric patterns that can be superimposed:

Vertical Bands
Horizontal Bands
Diagonal Bands
Diagonal Bands (Alternate Direction)
Arms
Rings
Spirals
Spirals (Alternatve Direction)

The parameters beginning with a lowercase 's' (such as sVert and sHorizon) are sliders that superimpose geometries.  The period parameters control the speed of the pattern, and the num parameters control the number of rings / arms / lines in the pattern.
