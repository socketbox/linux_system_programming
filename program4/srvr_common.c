
/*
 * receive client_id byte from client and verify
 *
 */
int check_client(cxfd)
{
  char preamble[PREAMB_LEN];
  if(recv(

