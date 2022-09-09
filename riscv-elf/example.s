
riscv-elf/example.riscv:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100e8 <exit>:
   100e8:	ff010113          	addi	sp,sp,-16
   100ec:	00000593          	li	a1,0
   100f0:	00813023          	sd	s0,0(sp)
   100f4:	00113423          	sd	ra,8(sp)
   100f8:	00050413          	mv	s0,a0
   100fc:	398000ef          	jal	ra,10494 <__call_exitprocs>
   10100:	f481b503          	ld	a0,-184(gp) # 11e88 <_global_impure_ptr>
   10104:	05853783          	ld	a5,88(a0)
   10108:	00078463          	beqz	a5,10110 <exit+0x28>
   1010c:	000780e7          	jalr	a5
   10110:	00040513          	mv	a0,s0
   10114:	5c0000ef          	jal	ra,106d4 <_exit>

0000000000010118 <register_fini>:
   10118:	00000793          	li	a5,0
   1011c:	00078863          	beqz	a5,1012c <register_fini+0x14>
   10120:	00010537          	lui	a0,0x10
   10124:	5d050513          	addi	a0,a0,1488 # 105d0 <__libc_fini_array>
   10128:	4940006f          	j	105bc <atexit>
   1012c:	00008067          	ret

0000000000010130 <_start>:
   10130:	00002197          	auipc	gp,0x2
   10134:	e1018193          	addi	gp,gp,-496 # 11f40 <__global_pointer$>
   10138:	f6018513          	addi	a0,gp,-160 # 11ea0 <completed.1>
   1013c:	f9818613          	addi	a2,gp,-104 # 11ed8 <__BSS_END__>
   10140:	40a60633          	sub	a2,a2,a0
   10144:	00000593          	li	a1,0
   10148:	270000ef          	jal	ra,103b8 <memset>
   1014c:	00000517          	auipc	a0,0x0
   10150:	47050513          	addi	a0,a0,1136 # 105bc <atexit>
   10154:	00050863          	beqz	a0,10164 <_start+0x34>
   10158:	00000517          	auipc	a0,0x0
   1015c:	47850513          	addi	a0,a0,1144 # 105d0 <__libc_fini_array>
   10160:	45c000ef          	jal	ra,105bc <atexit>
   10164:	1b8000ef          	jal	ra,1031c <__libc_init_array>
   10168:	00012503          	lw	a0,0(sp)
   1016c:	00810593          	addi	a1,sp,8
   10170:	00000613          	li	a2,0
   10174:	170000ef          	jal	ra,102e4 <main>
   10178:	f71ff06f          	j	100e8 <exit>

000000000001017c <__do_global_dtors_aux>:
   1017c:	ff010113          	addi	sp,sp,-16
   10180:	00813023          	sd	s0,0(sp)
   10184:	f601c783          	lbu	a5,-160(gp) # 11ea0 <completed.1>
   10188:	00113423          	sd	ra,8(sp)
   1018c:	02079263          	bnez	a5,101b0 <__do_global_dtors_aux+0x34>
   10190:	00000793          	li	a5,0
   10194:	00078a63          	beqz	a5,101a8 <__do_global_dtors_aux+0x2c>
   10198:	00011537          	lui	a0,0x11
   1019c:	72050513          	addi	a0,a0,1824 # 11720 <__FRAME_END__>
   101a0:	00000097          	auipc	ra,0x0
   101a4:	000000e7          	jalr	zero # 0 <exit-0x100e8>
   101a8:	00100793          	li	a5,1
   101ac:	f6f18023          	sb	a5,-160(gp) # 11ea0 <completed.1>
   101b0:	00813083          	ld	ra,8(sp)
   101b4:	00013403          	ld	s0,0(sp)
   101b8:	01010113          	addi	sp,sp,16
   101bc:	00008067          	ret

00000000000101c0 <frame_dummy>:
   101c0:	00000793          	li	a5,0
   101c4:	00078c63          	beqz	a5,101dc <frame_dummy+0x1c>
   101c8:	00011537          	lui	a0,0x11
   101cc:	f6818593          	addi	a1,gp,-152 # 11ea8 <object.0>
   101d0:	72050513          	addi	a0,a0,1824 # 11720 <__FRAME_END__>
   101d4:	00000317          	auipc	t1,0x0
   101d8:	00000067          	jr	zero # 0 <exit-0x100e8>
   101dc:	00008067          	ret

00000000000101e0 <print_d>:
   101e0:	fe010113          	addi	sp,sp,-32
   101e4:	00813c23          	sd	s0,24(sp)
   101e8:	02010413          	addi	s0,sp,32
   101ec:	00050793          	mv	a5,a0
   101f0:	fef42623          	sw	a5,-20(s0)
   101f4:	00200893          	li	a7,2
   101f8:	00000073          	ecall
   101fc:	00000013          	nop
   10200:	01813403          	ld	s0,24(sp)
   10204:	02010113          	addi	sp,sp,32
   10208:	00008067          	ret

000000000001020c <print_s>:
   1020c:	fe010113          	addi	sp,sp,-32
   10210:	00813c23          	sd	s0,24(sp)
   10214:	02010413          	addi	s0,sp,32
   10218:	fea43423          	sd	a0,-24(s0)
   1021c:	00000893          	li	a7,0
   10220:	00000073          	ecall
   10224:	00000013          	nop
   10228:	01813403          	ld	s0,24(sp)
   1022c:	02010113          	addi	sp,sp,32
   10230:	00008067          	ret

0000000000010234 <print_c>:
   10234:	fe010113          	addi	sp,sp,-32
   10238:	00813c23          	sd	s0,24(sp)
   1023c:	02010413          	addi	s0,sp,32
   10240:	00050793          	mv	a5,a0
   10244:	fef407a3          	sb	a5,-17(s0)
   10248:	00100893          	li	a7,1
   1024c:	00000073          	ecall
   10250:	00000013          	nop
   10254:	01813403          	ld	s0,24(sp)
   10258:	02010113          	addi	sp,sp,32
   1025c:	00008067          	ret

0000000000010260 <exit_proc>:
   10260:	ff010113          	addi	sp,sp,-16
   10264:	00813423          	sd	s0,8(sp)
   10268:	01010413          	addi	s0,sp,16
   1026c:	00300893          	li	a7,3
   10270:	00000073          	ecall
   10274:	00000013          	nop
   10278:	00813403          	ld	s0,8(sp)
   1027c:	01010113          	addi	sp,sp,16
   10280:	00008067          	ret

0000000000010284 <read_char>:
   10284:	fe010113          	addi	sp,sp,-32
   10288:	00813c23          	sd	s0,24(sp)
   1028c:	02010413          	addi	s0,sp,32
   10290:	00400893          	li	a7,4
   10294:	00000073          	ecall
   10298:	00050793          	mv	a5,a0
   1029c:	fef407a3          	sb	a5,-17(s0)
   102a0:	fef44783          	lbu	a5,-17(s0)
   102a4:	00078513          	mv	a0,a5
   102a8:	01813403          	ld	s0,24(sp)
   102ac:	02010113          	addi	sp,sp,32
   102b0:	00008067          	ret

00000000000102b4 <read_num>:
   102b4:	fe010113          	addi	sp,sp,-32
   102b8:	00813c23          	sd	s0,24(sp)
   102bc:	02010413          	addi	s0,sp,32
   102c0:	00500893          	li	a7,5
   102c4:	00000073          	ecall
   102c8:	00050793          	mv	a5,a0
   102cc:	fef43423          	sd	a5,-24(s0)
   102d0:	fe843783          	ld	a5,-24(s0)
   102d4:	00078513          	mv	a0,a5
   102d8:	01813403          	ld	s0,24(sp)
   102dc:	02010113          	addi	sp,sp,32
   102e0:	00008067          	ret

00000000000102e4 <main>:
   102e4:	ff010113          	addi	sp,sp,-16
   102e8:	00113423          	sd	ra,8(sp)
   102ec:	00813023          	sd	s0,0(sp)
   102f0:	01010413          	addi	s0,sp,16
   102f4:	000107b7          	lui	a5,0x10
   102f8:	71078513          	addi	a0,a5,1808 # 10710 <__errno+0xc>
   102fc:	f11ff0ef          	jal	ra,1020c <print_s>
   10300:	f61ff0ef          	jal	ra,10260 <exit_proc>
   10304:	00000793          	li	a5,0
   10308:	00078513          	mv	a0,a5
   1030c:	00813083          	ld	ra,8(sp)
   10310:	00013403          	ld	s0,0(sp)
   10314:	01010113          	addi	sp,sp,16
   10318:	00008067          	ret

000000000001031c <__libc_init_array>:
   1031c:	fe010113          	addi	sp,sp,-32
   10320:	00813823          	sd	s0,16(sp)
   10324:	000117b7          	lui	a5,0x11
   10328:	00011437          	lui	s0,0x11
   1032c:	01213023          	sd	s2,0(sp)
   10330:	72478793          	addi	a5,a5,1828 # 11724 <__preinit_array_end>
   10334:	72440713          	addi	a4,s0,1828 # 11724 <__preinit_array_end>
   10338:	00113c23          	sd	ra,24(sp)
   1033c:	00913423          	sd	s1,8(sp)
   10340:	40e78933          	sub	s2,a5,a4
   10344:	02e78263          	beq	a5,a4,10368 <__libc_init_array+0x4c>
   10348:	40395913          	srai	s2,s2,0x3
   1034c:	72440413          	addi	s0,s0,1828
   10350:	00000493          	li	s1,0
   10354:	00043783          	ld	a5,0(s0)
   10358:	00148493          	addi	s1,s1,1
   1035c:	00840413          	addi	s0,s0,8
   10360:	000780e7          	jalr	a5
   10364:	ff24e8e3          	bltu	s1,s2,10354 <__libc_init_array+0x38>
   10368:	00011437          	lui	s0,0x11
   1036c:	000117b7          	lui	a5,0x11
   10370:	73878793          	addi	a5,a5,1848 # 11738 <__do_global_dtors_aux_fini_array_entry>
   10374:	72840713          	addi	a4,s0,1832 # 11728 <__init_array_start>
   10378:	40e78933          	sub	s2,a5,a4
   1037c:	40395913          	srai	s2,s2,0x3
   10380:	02e78063          	beq	a5,a4,103a0 <__libc_init_array+0x84>
   10384:	72840413          	addi	s0,s0,1832
   10388:	00000493          	li	s1,0
   1038c:	00043783          	ld	a5,0(s0)
   10390:	00148493          	addi	s1,s1,1
   10394:	00840413          	addi	s0,s0,8
   10398:	000780e7          	jalr	a5
   1039c:	ff24e8e3          	bltu	s1,s2,1038c <__libc_init_array+0x70>
   103a0:	01813083          	ld	ra,24(sp)
   103a4:	01013403          	ld	s0,16(sp)
   103a8:	00813483          	ld	s1,8(sp)
   103ac:	00013903          	ld	s2,0(sp)
   103b0:	02010113          	addi	sp,sp,32
   103b4:	00008067          	ret

00000000000103b8 <memset>:
   103b8:	00f00313          	li	t1,15
   103bc:	00050713          	mv	a4,a0
   103c0:	02c37a63          	bgeu	t1,a2,103f4 <memset+0x3c>
   103c4:	00f77793          	andi	a5,a4,15
   103c8:	0a079063          	bnez	a5,10468 <memset+0xb0>
   103cc:	06059e63          	bnez	a1,10448 <memset+0x90>
   103d0:	ff067693          	andi	a3,a2,-16
   103d4:	00f67613          	andi	a2,a2,15
   103d8:	00e686b3          	add	a3,a3,a4
   103dc:	00b73023          	sd	a1,0(a4)
   103e0:	00b73423          	sd	a1,8(a4)
   103e4:	01070713          	addi	a4,a4,16
   103e8:	fed76ae3          	bltu	a4,a3,103dc <memset+0x24>
   103ec:	00061463          	bnez	a2,103f4 <memset+0x3c>
   103f0:	00008067          	ret
   103f4:	40c306b3          	sub	a3,t1,a2
   103f8:	00269693          	slli	a3,a3,0x2
   103fc:	00000297          	auipc	t0,0x0
   10400:	005686b3          	add	a3,a3,t0
   10404:	00c68067          	jr	12(a3)
   10408:	00b70723          	sb	a1,14(a4)
   1040c:	00b706a3          	sb	a1,13(a4)
   10410:	00b70623          	sb	a1,12(a4)
   10414:	00b705a3          	sb	a1,11(a4)
   10418:	00b70523          	sb	a1,10(a4)
   1041c:	00b704a3          	sb	a1,9(a4)
   10420:	00b70423          	sb	a1,8(a4)
   10424:	00b703a3          	sb	a1,7(a4)
   10428:	00b70323          	sb	a1,6(a4)
   1042c:	00b702a3          	sb	a1,5(a4)
   10430:	00b70223          	sb	a1,4(a4)
   10434:	00b701a3          	sb	a1,3(a4)
   10438:	00b70123          	sb	a1,2(a4)
   1043c:	00b700a3          	sb	a1,1(a4)
   10440:	00b70023          	sb	a1,0(a4)
   10444:	00008067          	ret
   10448:	0ff5f593          	zext.b	a1,a1
   1044c:	00859693          	slli	a3,a1,0x8
   10450:	00d5e5b3          	or	a1,a1,a3
   10454:	01059693          	slli	a3,a1,0x10
   10458:	00d5e5b3          	or	a1,a1,a3
   1045c:	02059693          	slli	a3,a1,0x20
   10460:	00d5e5b3          	or	a1,a1,a3
   10464:	f6dff06f          	j	103d0 <memset+0x18>
   10468:	00279693          	slli	a3,a5,0x2
   1046c:	00000297          	auipc	t0,0x0
   10470:	005686b3          	add	a3,a3,t0
   10474:	00008293          	mv	t0,ra
   10478:	f98680e7          	jalr	-104(a3)
   1047c:	00028093          	mv	ra,t0
   10480:	ff078793          	addi	a5,a5,-16
   10484:	40f70733          	sub	a4,a4,a5
   10488:	00f60633          	add	a2,a2,a5
   1048c:	f6c374e3          	bgeu	t1,a2,103f4 <memset+0x3c>
   10490:	f3dff06f          	j	103cc <memset+0x14>

0000000000010494 <__call_exitprocs>:
   10494:	fb010113          	addi	sp,sp,-80
   10498:	03413023          	sd	s4,32(sp)
   1049c:	f481ba03          	ld	s4,-184(gp) # 11e88 <_global_impure_ptr>
   104a0:	03213823          	sd	s2,48(sp)
   104a4:	04113423          	sd	ra,72(sp)
   104a8:	1f8a3903          	ld	s2,504(s4)
   104ac:	04813023          	sd	s0,64(sp)
   104b0:	02913c23          	sd	s1,56(sp)
   104b4:	03313423          	sd	s3,40(sp)
   104b8:	01513c23          	sd	s5,24(sp)
   104bc:	01613823          	sd	s6,16(sp)
   104c0:	01713423          	sd	s7,8(sp)
   104c4:	01813023          	sd	s8,0(sp)
   104c8:	04090063          	beqz	s2,10508 <__call_exitprocs+0x74>
   104cc:	00050b13          	mv	s6,a0
   104d0:	00058b93          	mv	s7,a1
   104d4:	00100a93          	li	s5,1
   104d8:	fff00993          	li	s3,-1
   104dc:	00892483          	lw	s1,8(s2)
   104e0:	fff4841b          	addiw	s0,s1,-1
   104e4:	02044263          	bltz	s0,10508 <__call_exitprocs+0x74>
   104e8:	00349493          	slli	s1,s1,0x3
   104ec:	009904b3          	add	s1,s2,s1
   104f0:	040b8463          	beqz	s7,10538 <__call_exitprocs+0xa4>
   104f4:	2084b783          	ld	a5,520(s1)
   104f8:	05778063          	beq	a5,s7,10538 <__call_exitprocs+0xa4>
   104fc:	fff4041b          	addiw	s0,s0,-1
   10500:	ff848493          	addi	s1,s1,-8
   10504:	ff3416e3          	bne	s0,s3,104f0 <__call_exitprocs+0x5c>
   10508:	04813083          	ld	ra,72(sp)
   1050c:	04013403          	ld	s0,64(sp)
   10510:	03813483          	ld	s1,56(sp)
   10514:	03013903          	ld	s2,48(sp)
   10518:	02813983          	ld	s3,40(sp)
   1051c:	02013a03          	ld	s4,32(sp)
   10520:	01813a83          	ld	s5,24(sp)
   10524:	01013b03          	ld	s6,16(sp)
   10528:	00813b83          	ld	s7,8(sp)
   1052c:	00013c03          	ld	s8,0(sp)
   10530:	05010113          	addi	sp,sp,80
   10534:	00008067          	ret
   10538:	00892783          	lw	a5,8(s2)
   1053c:	0084b703          	ld	a4,8(s1)
   10540:	fff7879b          	addiw	a5,a5,-1
   10544:	06878263          	beq	a5,s0,105a8 <__call_exitprocs+0x114>
   10548:	0004b423          	sd	zero,8(s1)
   1054c:	fa0708e3          	beqz	a4,104fc <__call_exitprocs+0x68>
   10550:	31092783          	lw	a5,784(s2)
   10554:	008a96bb          	sllw	a3,s5,s0
   10558:	00892c03          	lw	s8,8(s2)
   1055c:	00d7f7b3          	and	a5,a5,a3
   10560:	0007879b          	sext.w	a5,a5
   10564:	02079263          	bnez	a5,10588 <__call_exitprocs+0xf4>
   10568:	000700e7          	jalr	a4
   1056c:	00892703          	lw	a4,8(s2)
   10570:	1f8a3783          	ld	a5,504(s4)
   10574:	01871463          	bne	a4,s8,1057c <__call_exitprocs+0xe8>
   10578:	f92782e3          	beq	a5,s2,104fc <__call_exitprocs+0x68>
   1057c:	f80786e3          	beqz	a5,10508 <__call_exitprocs+0x74>
   10580:	00078913          	mv	s2,a5
   10584:	f59ff06f          	j	104dc <__call_exitprocs+0x48>
   10588:	31492783          	lw	a5,788(s2)
   1058c:	1084b583          	ld	a1,264(s1)
   10590:	00d7f7b3          	and	a5,a5,a3
   10594:	0007879b          	sext.w	a5,a5
   10598:	00079c63          	bnez	a5,105b0 <__call_exitprocs+0x11c>
   1059c:	000b0513          	mv	a0,s6
   105a0:	000700e7          	jalr	a4
   105a4:	fc9ff06f          	j	1056c <__call_exitprocs+0xd8>
   105a8:	00892423          	sw	s0,8(s2)
   105ac:	fa1ff06f          	j	1054c <__call_exitprocs+0xb8>
   105b0:	00058513          	mv	a0,a1
   105b4:	000700e7          	jalr	a4
   105b8:	fb5ff06f          	j	1056c <__call_exitprocs+0xd8>

00000000000105bc <atexit>:
   105bc:	00050593          	mv	a1,a0
   105c0:	00000693          	li	a3,0
   105c4:	00000613          	li	a2,0
   105c8:	00000513          	li	a0,0
   105cc:	0600006f          	j	1062c <__register_exitproc>

00000000000105d0 <__libc_fini_array>:
   105d0:	fe010113          	addi	sp,sp,-32
   105d4:	00813823          	sd	s0,16(sp)
   105d8:	000117b7          	lui	a5,0x11
   105dc:	00011437          	lui	s0,0x11
   105e0:	73878793          	addi	a5,a5,1848 # 11738 <__do_global_dtors_aux_fini_array_entry>
   105e4:	74040413          	addi	s0,s0,1856 # 11740 <impure_data>
   105e8:	40f40433          	sub	s0,s0,a5
   105ec:	00913423          	sd	s1,8(sp)
   105f0:	00113c23          	sd	ra,24(sp)
   105f4:	40345493          	srai	s1,s0,0x3
   105f8:	02048063          	beqz	s1,10618 <__libc_fini_array+0x48>
   105fc:	ff840413          	addi	s0,s0,-8
   10600:	00f40433          	add	s0,s0,a5
   10604:	00043783          	ld	a5,0(s0)
   10608:	fff48493          	addi	s1,s1,-1
   1060c:	ff840413          	addi	s0,s0,-8
   10610:	000780e7          	jalr	a5
   10614:	fe0498e3          	bnez	s1,10604 <__libc_fini_array+0x34>
   10618:	01813083          	ld	ra,24(sp)
   1061c:	01013403          	ld	s0,16(sp)
   10620:	00813483          	ld	s1,8(sp)
   10624:	02010113          	addi	sp,sp,32
   10628:	00008067          	ret

000000000001062c <__register_exitproc>:
   1062c:	f481b703          	ld	a4,-184(gp) # 11e88 <_global_impure_ptr>
   10630:	1f873783          	ld	a5,504(a4)
   10634:	06078063          	beqz	a5,10694 <__register_exitproc+0x68>
   10638:	0087a703          	lw	a4,8(a5)
   1063c:	01f00813          	li	a6,31
   10640:	08e84663          	blt	a6,a4,106cc <__register_exitproc+0xa0>
   10644:	02050863          	beqz	a0,10674 <__register_exitproc+0x48>
   10648:	00371813          	slli	a6,a4,0x3
   1064c:	01078833          	add	a6,a5,a6
   10650:	10c83823          	sd	a2,272(a6)
   10654:	3107a883          	lw	a7,784(a5)
   10658:	00100613          	li	a2,1
   1065c:	00e6163b          	sllw	a2,a2,a4
   10660:	00c8e8b3          	or	a7,a7,a2
   10664:	3117a823          	sw	a7,784(a5)
   10668:	20d83823          	sd	a3,528(a6)
   1066c:	00200693          	li	a3,2
   10670:	02d50863          	beq	a0,a3,106a0 <__register_exitproc+0x74>
   10674:	00270693          	addi	a3,a4,2
   10678:	00369693          	slli	a3,a3,0x3
   1067c:	0017071b          	addiw	a4,a4,1
   10680:	00e7a423          	sw	a4,8(a5)
   10684:	00d787b3          	add	a5,a5,a3
   10688:	00b7b023          	sd	a1,0(a5)
   1068c:	00000513          	li	a0,0
   10690:	00008067          	ret
   10694:	20070793          	addi	a5,a4,512
   10698:	1ef73c23          	sd	a5,504(a4)
   1069c:	f9dff06f          	j	10638 <__register_exitproc+0xc>
   106a0:	3147a683          	lw	a3,788(a5)
   106a4:	00000513          	li	a0,0
   106a8:	00c6e6b3          	or	a3,a3,a2
   106ac:	30d7aa23          	sw	a3,788(a5)
   106b0:	00270693          	addi	a3,a4,2
   106b4:	00369693          	slli	a3,a3,0x3
   106b8:	0017071b          	addiw	a4,a4,1
   106bc:	00e7a423          	sw	a4,8(a5)
   106c0:	00d787b3          	add	a5,a5,a3
   106c4:	00b7b023          	sd	a1,0(a5)
   106c8:	00008067          	ret
   106cc:	fff00513          	li	a0,-1
   106d0:	00008067          	ret

00000000000106d4 <_exit>:
   106d4:	05d00893          	li	a7,93
   106d8:	00000073          	ecall
   106dc:	00054463          	bltz	a0,106e4 <_exit+0x10>
   106e0:	0000006f          	j	106e0 <_exit+0xc>
   106e4:	ff010113          	addi	sp,sp,-16
   106e8:	00813023          	sd	s0,0(sp)
   106ec:	00050413          	mv	s0,a0
   106f0:	00113423          	sd	ra,8(sp)
   106f4:	4080043b          	negw	s0,s0
   106f8:	00c000ef          	jal	ra,10704 <__errno>
   106fc:	00852023          	sw	s0,0(a0)
   10700:	0000006f          	j	10700 <_exit+0x2c>

0000000000010704 <__errno>:
   10704:	f581b503          	ld	a0,-168(gp) # 11e98 <_impure_ptr>
   10708:	00008067          	ret

Disassembly of section .rodata:

0000000000010710 <.rodata>:
   10710:	6548                	.2byte	0x6548
   10712:	6c6c                	.2byte	0x6c6c
   10714:	57202c6f          	jal	s8,12c86 <__global_pointer$+0xd46>
   10718:	646c726f          	jal	tp,d7d5e <__global_pointer$+0xc5e1e>
   1071c:	0a21                	.2byte	0xa21
	...

Disassembly of section .eh_frame:

0000000000011720 <__FRAME_END__>:
   11720:	0000                	.2byte	0x0
	...

Disassembly of section .init_array:

0000000000011728 <__init_array_start>:
   11728:	0118                	.2byte	0x118
   1172a:	0001                	.2byte	0x1
   1172c:	0000                	.2byte	0x0
	...

0000000000011730 <__frame_dummy_init_array_entry>:
   11730:	01c0                	.2byte	0x1c0
   11732:	0001                	.2byte	0x1
   11734:	0000                	.2byte	0x0
	...

Disassembly of section .fini_array:

0000000000011738 <__do_global_dtors_aux_fini_array_entry>:
   11738:	017c                	.2byte	0x17c
   1173a:	0001                	.2byte	0x1
   1173c:	0000                	.2byte	0x0
	...

Disassembly of section .data:

0000000000011740 <impure_data>:
	...
   11748:	1c78                	.2byte	0x1c78
   1174a:	0001                	.2byte	0x1
   1174c:	0000                	.2byte	0x0
   1174e:	0000                	.2byte	0x0
   11750:	1d28                	.2byte	0x1d28
   11752:	0001                	.2byte	0x1
   11754:	0000                	.2byte	0x0
   11756:	0000                	.2byte	0x0
   11758:	1dd8                	.2byte	0x1dd8
   1175a:	0001                	.2byte	0x1
	...
   11828:	0001                	.2byte	0x1
   1182a:	0000                	.2byte	0x0
   1182c:	0000                	.2byte	0x0
   1182e:	0000                	.2byte	0x0
   11830:	330e                	.2byte	0x330e
   11832:	abcd                	.2byte	0xabcd
   11834:	1234                	.2byte	0x1234
   11836:	e66d                	.2byte	0xe66d
   11838:	deec                	.2byte	0xdeec
   1183a:	0005                	.2byte	0x5
   1183c:	0000000b          	.4byte	0xb
	...

Disassembly of section .sdata:

0000000000011e88 <_global_impure_ptr>:
   11e88:	1740                	.2byte	0x1740
   11e8a:	0001                	.2byte	0x1
   11e8c:	0000                	.2byte	0x0
	...

0000000000011e90 <__dso_handle>:
	...

0000000000011e98 <_impure_ptr>:
   11e98:	1740                	.2byte	0x1740
   11e9a:	0001                	.2byte	0x1
   11e9c:	0000                	.2byte	0x0
	...

Disassembly of section .bss:

0000000000011ea0 <completed.1>:
	...

0000000000011ea8 <object.0>:
	...

Disassembly of section .comment:

0000000000000000 <.comment>:
   0:	3a434347          	.4byte	0x3a434347
   4:	2820                	.2byte	0x2820
   6:	61653167          	.4byte	0x61653167
   a:	3739                	.2byte	0x3739
   c:	6538                	.2byte	0x6538
   e:	36363033          	.4byte	0x36363033
  12:	2029                	.2byte	0x2029
  14:	3231                	.2byte	0x3231
  16:	312e                	.2byte	0x312e
  18:	302e                	.2byte	0x302e
  1a:	4700                	.2byte	0x4700
  1c:	203a4343          	.4byte	0x203a4343
  20:	4728                	.2byte	0x4728
  22:	554e                	.2byte	0x554e
  24:	2029                	.2byte	0x2029
  26:	3231                	.2byte	0x3231
  28:	312e                	.2byte	0x312e
  2a:	302e                	.2byte	0x302e
	...

Disassembly of section .riscv.attributes:

0000000000000000 <.riscv.attributes>:
   0:	2041                	.2byte	0x2041
   2:	0000                	.2byte	0x0
   4:	7200                	.2byte	0x7200
   6:	7369                	.2byte	0x7369
   8:	01007663          	bgeu	zero,a6,14 <exit-0x100d4>
   c:	0016                	.2byte	0x16
   e:	0000                	.2byte	0x0
  10:	1004                	.2byte	0x1004
  12:	7205                	.2byte	0x7205
  14:	3676                	.2byte	0x3676
  16:	6934                	.2byte	0x6934
  18:	7032                	.2byte	0x7032
  1a:	5f30                	.2byte	0x5f30
  1c:	3261                	.2byte	0x3261
  1e:	3070                	.2byte	0x3070
	...
