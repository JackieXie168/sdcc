/* avr.src/instcl.h */

  virtual int inst_nop(t_mem code);
  virtual int inst_ld(t_mem code);
  virtual int inst_inc(t_mem code);
  virtual int inst_dec(t_mem code);
  virtual int inst_rlca(t_mem code);
  virtual int inst_rrca(t_mem code);
  virtual int inst_ex(t_mem code);
  virtual int inst_add(t_mem code);
  virtual int inst_djnz(t_mem code);
  virtual int inst_jr(t_mem code);
  virtual int inst_rla(t_mem code);
  virtual int inst_rra(t_mem code);
  virtual int inst_daa(t_mem code);
  virtual int inst_cpl(t_mem code);
  virtual int inst_scf(t_mem code);
  virtual int inst_ccf(t_mem code);
  virtual int inst_halt(t_mem code);
  virtual int inst_adc(t_mem code);
  virtual int inst_sbc(t_mem code);
  virtual int inst_and(t_mem code);
  virtual int inst_xor(t_mem code);
  virtual int inst_or(t_mem code);
  virtual int inst_cp(t_mem code);
  virtual int inst_rst(t_mem code);
  virtual int inst_ret(t_mem code);
  virtual int inst_call(t_mem code);
  virtual int inst_out(t_mem code);
  virtual int inst_push(t_mem code);
  virtual int inst_exx(t_mem code);
  virtual int inst_in(t_mem code);
  virtual int inst_sub(t_mem code);
  virtual int inst_pop(t_mem code);
  virtual int inst_jp(t_mem code);
  virtual int inst_di(t_mem code);
  virtual int inst_ei(t_mem code);

/* End of avr.src/instcl.h */
