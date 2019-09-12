<strong>Project Created By:</strong>
<ul>
<li> Se Hoon Bang </li>
<li> Kenneth Siu </li>
</ul>

<h1> rshell </h1>
rshell is a shell used to run commands and connectors

<h2> Features </h2>
<h4> Executables </h4>
Run any comands, besides cd, such as ls, pwd, echo, etc.
<br>  e.g.:  ls will list the contents of the current directory
Run "exit" to exit rshell.
<h4> Connectors </h4>
Use connectors, such as ||, &&, and ; to connect commands however you like.
<br>  e.g.: (a && b || c) will run ((a && b) || c) instead of (a && (b || c)))
<h4> Comments </h4>
Use # to commment out any part of a command.
<br>  e.g.: echo hello world # hello world is printed, but this is not!

<h2> Implementation </h2>
The project consisted of several steps.
<ul>
  <li> We got the user input using getline, and parced it into three different vectors - executables, arguments, and connectors. </li>
  <li> We called execvp with the executable (and argument) as parameters. </li>
  <li> We checked whether or not the executable (and argument) was valid and if it was, ran it. </li>
  <li> We had a function to check whether or not the next executable should be run depending on the connectors. </li>
  <li> We had another function which calculates the proper grouping of commands. (e.g.: if we ran a && b || c, it should run ( (a && b) || c ) instead of ( a && (b || c) )). </li>
  <li> We put the whole thing into a while loop so that it would run unless the executable entered is "exit" in which it would then exit the rshell. </li>
  <li> We tested the rshell by outputting our expected output, followed by outputting our actual output. </li>
</ul>

<h2> Possible Bugs & Future Updates </h2>
The cd command has not been implemented.
