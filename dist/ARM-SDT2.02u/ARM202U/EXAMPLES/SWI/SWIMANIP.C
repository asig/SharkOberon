unsigned __swi_indirect(0x80)
    SWI_ManipulateObject(unsigned operationNumber, unsigned object,
                         unsigned parameter);

unsigned DoSelectedManipulation(unsigned object, unsigned parameter,
                            unsigned operation)
{
  return SWI_ManipulateObject(operation, object, parameter);
}
