/*@1@*/

#include <stdlib.h>
#include "i_string.h"

// sim
#include "simifcl.h"


/* Interface command */

cl_sif_command::cl_sif_command(enum sif_command cmd, char *the_description,
			       class cl_simulator_interface *the_sif):
  cl_base()
{
  command= cmd;
  description= strdup(the_description);
  sif= the_sif;
  parameters= 0;
  answer= 0;
  nuof_params= params_received= 0;
  answer_length= answered_bytes= 0;
  answering= DD_FALSE;
}

cl_sif_command::~cl_sif_command(void)
{
  if (description)
    free(description);
  clear_params();
  clear_answer();
}

int
cl_sif_command::init(void)
{
  return(0);
}

void
cl_sif_command::clear_params(void)
{
  nuof_params= 0;
  params_received= 0;
  if (parameters)
    free(parameters);
}

void
cl_sif_command::clear_answer(void)
{
  answer_length= 0;
  answered_bytes= 0;
  if (answer)
    free(answer);
}

void
cl_sif_command::clear(void)
{
  clear_params();
  clear_answer();
}


bool
cl_sif_command::get_parameter(int nr, t_mem *into)
{
  if (!parameters ||
      nr >= nuof_params)
    return(DD_FALSE);
  if (into)
    *into= parameters[nr];
  return(DD_TRUE);
}


t_mem
cl_sif_command::read(class cl_memory_cell *cel)
{
  t_mem ret= cel->get();

  if (answering &&
      answer)
    {
      if (answered_bytes < answer_length)
	{
	  ret= answer[answered_bytes];
	  answered_bytes++;
	}
      if (answered_bytes >= answer_length)
	{
	  sif->finish_command();
	}
    }
  return(ret);
}

void
cl_sif_command::write(class cl_memory_cell *cel, t_mem *val)
{
  if (nuof_params &&
      params_received < nuof_params &&
      parameters)
    {
      parameters[params_received]= *val;
      params_received++;
      if (params_received >= nuof_params)
	{
	  produce_answer();
	  start_answer();
	}
    }
}


void
cl_sif_command::start(void)
{
  printf("Command %d started\n", command);
  need_params(0);
}

void
cl_sif_command::need_params(int nr)
{
  clear_params();
  if (nr <= 0)
    {
      produce_answer();
      start_answer();
    }
  nuof_params= nr;
  params_received= 0;
  parameters= (t_mem *)calloc(nr, sizeof(t_mem));
}

void
cl_sif_command::produce_answer(void)
{
  clear_answer();
  //produce_answer("ABC");
}

void
cl_sif_command::produce_answer(t_mem ans)
{
  clear_answer();
  answer= (t_mem *)calloc(1, sizeof(t_mem));
  answer[0]= ans;
  answer_length= 1;
}

void
cl_sif_command::produce_answer(char *ans)
{
  clear_answer();
  if (ans &&
      *ans)
    {
      answer= (t_mem *)calloc(strlen(ans)+1, sizeof(char));
      int i= 0;
      while (ans[i])
	{
	  answer[i]= ans[i];
	  i++;
	}
      answer[i]= '\0';
      answer_length= i+1;
    }
}

void
cl_sif_command::start_answer(void)
{
  if (answer_length)
    answering= DD_TRUE;
  else
    {
      answering= DD_FALSE;
      sif->finish_command();
    }
}


/*
 * Virtual HW: simulator interface
 */

cl_simulator_interface::cl_simulator_interface(class cl_uc *auc,
					       char *the_as_name,
					       t_addr the_addr):
  cl_hw(auc, HW_SIMIF, 0, "simif")
{
  version= 1;
  as_name= strdup(the_as_name);
  addr= the_addr;
  commands= new cl_list(2, 2, "sif_commands");
  active_command= 0;
}

cl_simulator_interface::~cl_simulator_interface(void)
{
  if (as_name)
    free(as_name);
  delete commands;
}

int
cl_simulator_interface::init(void)
{
  as= uc->address_space(as_name);
  if (as)
    {
      address= addr;
      if (addr < 0)
	address= as->highest_valid_address();
      register_cell(as, address, &cell, wtd_restore_write);
    }
  class cl_sif_command *c;
  commands->add(c= new cl_sif_command(SIFCM_IFVER,
				      "Get version of interface",
				      this));
  c->set_name("if_ver");
  c->init();
  return(0);
}


void
cl_simulator_interface::set_cmd(class cl_cmdline *cmdline,
				class cl_console *con)
{
  class cl_cmd_arg *params[2]= {
    cmdline->param(0),
    cmdline->param(1)
  };

  if (cmdline->syntax_match(uc, MEMORY ADDRESS))
    {
      class cl_memory *mem= params[0]->value.memory.memory;
      t_addr a= params[1]->value.address;
      if (!mem->is_address_space())
	{
	  con->dd_printf("%s is not an address space\n");
	  return;
	}
      if (!mem->valid_address(a))
	{
	  con->dd_printf("Address must be between 0x%x and 0x%x\n",
			 mem->lowest_valid_address(),
			 mem->highest_valid_address());
	  return;
	}
      as_name= mem->get_name();
      addr= a;
      if ((as= dynamic_cast<class cl_address_space *>(mem)) != 0)
	{
	  address= addr;
	  if (addr < 0)
	    address= as->highest_valid_address();
	  register_cell(as, address, &cell, wtd_restore_write);
	}
    }
  else
    {
      con->dd_printf("set hardware simif memory address\n");
    }
}


t_mem
cl_simulator_interface::read(class cl_memory_cell *cel)
{
  printf("simif read\n");
  if (!active_command)
    {
      t_mem d= cel->get();
      return(~d & cel->get_mask());
    }
  else
    return(active_command->read(cel));
  return(cel->get());
}

void
cl_simulator_interface::write(class cl_memory_cell *cel, t_mem *val)
{
  printf("simif write %d 0x%x\n",*val,*val);
  if (!active_command)
    {
      int i;
      for (i= 0; i < commands->count; i++)
	{
	  class cl_sif_command *c=
	    dynamic_cast<class cl_sif_command *>(commands->object_at(i));
	  if (!c)
	    continue;
	  if (*val == c->get_command())
	    {
	      active_command= c;
	      c->start();
	      return;
	    }
	}
    }
  else
    active_command->write(cel, val);
}


void
cl_simulator_interface::finish_command(void)
{
  if (active_command)
    printf("Command %d finished\n", active_command->get_command());
  else
    printf("Command finished\n");
  active_command= 0;
}


void
cl_simulator_interface::print_info(class cl_console *con)
{
  con->dd_printf("uCsim simulator interface, version %d, ", version);
  con->dd_printf("at %s[", as_name);
  if (as)
    con->dd_printf(as->addr_format, address);
  else
    con->dd_printf("0x%x", address);
  con->dd_printf("]\n");
  
  con->dd_printf("Active command: ");
  if (!active_command)
    con->dd_printf("none.\n");
  else
    {
      class cl_sif_command *c= active_command;
      con->dd_printf("0x%02x %s %s\n", c->get_command(),
		     c->get_name(), c->get_description());
      con->dd_printf("Parameters received %d bytes of %d\n",
		     c->get_params_received(), c->get_nuof_params());
      con->dd_printf(" ");
      int i;
      for (i= 0; i < c->get_nuof_params(); i++)
	{
	  t_mem p;
	  if (c->get_parameter(i, &p))
	    con->dd_printf(" %02x", p);
	  else
	    con->dd_printf(" --");
	}
      con->dd_printf("\n");
      con->dd_printf("Answered %d bytes of %d\n",
		     c->get_answered_bytes(), c->get_answer_length());
      con->dd_printf("Answering: %s\n", (c->get_answering())?"yes":"no");
    }
  
  int i;
  con->dd_printf("Known commands:\n");
  for (i= 0; i < commands->count; i++)
    {
      class cl_sif_command *c=
	dynamic_cast<class cl_sif_command *>(commands->object_at(i));
      if (!c)
	continue;
      con->dd_printf("0x%02x %s %s\n", c->get_command(),
		     c->get_name(), c->get_description());
    }
}


/* End of sim.src/simif.cc */
