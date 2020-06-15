# Instructions

## How to import the image into VM and set it up?

  1. Download virtual box
  2. Download the image from the course website box link
  3. Open the virtual box and hit new to create a new VM.
  4. Type in any name you want in the "name" field
  5. Then choose "Linux" in the "Type" drop-down.
  6. Then, in the "Version" drop-down choose "Fedora (64 bit)"
  7. Hit "Continue"
  8. Choose the Memory Size that you'd like to allocate. Hit "Continue".
  9. Select "Use an existing virtual hard disk file" and then browse and select the image file that you downloaded from the course web-page.
  10. Then you should be able to launch the VM from the list that is displayed when you launch VBox.
  11. *Only do this step if you are not using the provided VM Image* - You need to run the script `bitrate_project_starter/setup.sh`. You may need to change the permissions first `chmod u+x setup.sh"`
  12. If you want to ssh into your VM, you need to change your network setting on VM to "Bridged Adapter"
  13. The VM doesn't have yacc installed. You need to type "sudo yum install byacc" in terminal to install it manually

##How run the program and watch the video from the browser?

  1. Please check sec 2.1 for general guidance.
  2. You need to run network simulation as specified in writeup section 4.3
  3. You need to run your proxy as sepcified in writeup section 2.2.4
  4. Then you can open the firefox, type in the url specified in 2.2.4, and debug from there!
