/*@1@*/

#ifndef SIMIFCL_HEADER
#define SIMIFCL_HEADER

#include "uccl.h"


enum sif_command {
  SIFCM_IFVER		= 0x01,	// interface version
  SIFCM_SIMVER		= 0x02, // simulator version
  SIFCM_IFRESET		= 0x03  // reset the interface
};

class cl_simulator_interface;

class cl_sif_command: public cl_base
{
private:
  enum sif_command command;
  char *description;
  class cl_simulator_interface *sif;
  t_mem *parameters;
  int nuof_params, params_received;
  t_mem *answer;
  int answer_length, answered_bytes;
  bool answering;
public:
  cl_sif_command(enum sif_command cmd, char *the_description,
		 class cl_simulator_interface *the_sif);
  virtual ~cl_sif_command(void);
  virtual int init(void);
  virtual void clear_params(void);
  virtual void clear_answer(void);
  virtual void clear(void);

  enum sif_command get_command(void) { return(command); }
  char *get_description(void) { return(description); }
  int get_nuof_params(void) { return(nuof_params); }
  int get_params_received(void) { return(params_received); }
  int get_answer_length(void) { return(answer_length); }
  int get_answered_bytes(void) { return(answered_bytes); }
  bool get_answering(void) { return(answering); }
  bool get_parameter(int nr, t_mem *into);

  virtual t_mem read(class cl_memory_cell *cel);
  virtual void write(class cl_memory_cell *cel, t_mem *val);

  virtual void start(void);
  virtual void need_params(int nr);
  virtual void produce_answer(void);
  virtual void produce_answer(t_mem ans);
  virtual void produce_answer(char *ans);
  virtual void start_answer(void);
};


class cl_simulator_interface: public cl_hw
{
private:
  int version;
  char *as_name;
  t_addr addr;
  class cl_address_space *as;
  t_addr address;
  class cl_memory_cell *cell;
  class cl_list *commands;
  class cl_sif_command *active_command;
public:
  cl_simulator_interface(class cl_uc *auc, char *the_as_name, t_addr the_addr);
  virtual ~cl_simulator_interface(void);
  virtual int init(void);

  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console *con);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  
  virtual void finish_command(void);

  virtual void print_info(class cl_console *con);
};


#endif
