The loader will
1.Get the update firmware start block (next block of the last block of execute image)
2.Check TAGs for update firmware
3.Load update firmware if TAGs are correctm otherwise load execute image.

Note: The execute image & update firmware must include the update operation (SampleCode\_HID_Updater).