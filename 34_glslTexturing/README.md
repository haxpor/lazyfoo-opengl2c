# Notes

* First sample that applies a new approach in creating a new struct's functionality. More flat way this time with snake case for file name and definitions.
* VAO needed only when rendering. Updating uniform/attribute variable to GPU need program to be bound first.
* Opengl error along the line of "The specified operation is not allowed in the current state" can be fixed by not using any fixed-function function, and can be tracked down by checking error returning from `glGetError` after a call to every single Opengl functions.
