
../riscv-elf/example.riscv:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100e8 <exit>:
   100e8:	ff010113          	addi	sp,sp,-16
   100ec:	00000593          	li	a1,0
   100f0:	00813023          	sd	s0,0(sp)
   100f4:	00113423          	sd	ra,8(sp)
   100f8:	00050413          	mv	s0,a0
   100fc:	3d8000ef          	jal	ra,104d4 <__call_exitprocs>
   10100:	f481b503          	ld	a0,-184(gp) # 11eb0 <_global_impure_ptr>
   10104:	05853783          	ld	a5,88(a0)
   10108:	00078463          	beqz	a5,10110 <exit+0x28>
   1010c:	000780e7          	jalr	a5
   10110:	00040513          	mv	a0,s0
   10114:	600000ef          	jal	ra,10714 <_exit>

0000000000010118 <register_fini>:
   10118:	00000793          	li	a5,0
   1011c:	00078863          	beqz	a5,1012c <register_fini+0x14>
   10120:	00010537          	lui	a0,0x10
   10124:	61050513          	addi	a0,a0,1552 # 10610 <__libc_fini_array>
   10128:	4d40006f          	j	105fc <atexit>
   1012c:	00008067          	ret

0000000000010130 <_start>:
   10130:	00002197          	auipc	gp,0x2
   10134:	e3818193          	addi	gp,gp,-456 # 11f68 <__global_pointer$>
   10138:	f6018513          	addi	a0,gp,-160 # 11ec8 <completed.1>
   1013c:	f9818613          	addi	a2,gp,-104 # 11f00 <__BSS_END__>
   10140:	40a60633          	sub	a2,a2,a0
   10144:	00000593          	li	a1,0
   10148:	2b0000ef          	jal	ra,103f8 <memset>
   1014c:	00000517          	auipc	a0,0x0
   10150:	4b050513          	addi	a0,a0,1200 # 105fc <atexit>
   10154:	00050863          	beqz	a0,10164 <_start+0x34>
   10158:	00000517          	auipc	a0,0x0
   1015c:	4b850513          	addi	a0,a0,1208 # 10610 <__libc_fini_array>
   10160:	49c000ef          	jal	ra,105fc <atexit>
   10164:	1f8000ef          	jal	ra,1035c <__libc_init_array>
   10168:	00012503          	lw	a0,0(sp)
   1016c:	00810593          	addi	a1,sp,8
   10170:	00000613          	li	a2,0
   10174:	170000ef          	jal	ra,102e4 <main>
   10178:	f71ff06f          	j	100e8 <exit>

000000000001017c <__do_global_dtors_aux>:
   1017c:	ff010113          	addi	sp,sp,-16
   10180:	00813023          	sd	s0,0(sp)
   10184:	f601c783          	lbu	a5,-160(gp) # 11ec8 <completed.1>
   10188:	00113423          	sd	ra,8(sp)
   1018c:	02079263          	bnez	a5,101b0 <__do_global_dtors_aux+0x34>
   10190:	00000793          	li	a5,0
   10194:	00078a63          	beqz	a5,101a8 <__do_global_dtors_aux+0x2c>
   10198:	00011537          	lui	a0,0x11
   1019c:	74c50513          	addi	a0,a0,1868 # 1174c <__FRAME_END__>
   101a0:	00000097          	auipc	ra,0x0
   101a4:	000000e7          	jalr	zero # 0 <exit-0x100e8>
   101a8:	00100793          	li	a5,1
   101ac:	f6f18023          	sb	a5,-160(gp) # 11ec8 <completed.1>
   101b0:	00813083          	ld	ra,8(sp)
   101b4:	00013403          	ld	s0,0(sp)
   101b8:	01010113          	addi	sp,sp,16
   101bc:	00008067          	ret

00000000000101c0 <frame_dummy>:
   101c0:	00000793          	li	a5,0
   101c4:	00078c63          	beqz	a5,101dc <frame_dummy+0x1c>
   101c8:	00011537          	lui	a0,0x11
   101cc:	f6818593          	addi	a1,gp,-152 # 11ed0 <object.0>
   101d0:	74c50513          	addi	a0,a0,1868 # 1174c <__FRAME_END__>
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
   102e4:	fd010113          	addi	sp,sp,-48
   102e8:	02113423          	sd	ra,40(sp)
   102ec:	02813023          	sd	s0,32(sp)
   102f0:	03010413          	addi	s0,sp,48
   102f4:	0001c7b7          	lui	a5,0x1c
   102f8:	f5278793          	addi	a5,a5,-174 # 1bf52 <__global_pointer$+0x9fea>
   102fc:	fcf43823          	sd	a5,-48(s0)
   10300:	fd040793          	addi	a5,s0,-48
   10304:	fef43423          	sd	a5,-24(s0)
   10308:	fe042223          	sw	zero,-28(s0)
   1030c:	fe042023          	sw	zero,-32(s0)
   10310:	00100793          	li	a5,1
   10314:	fcf42e23          	sw	a5,-36(s0)
   10318:	00300793          	li	a5,3
   1031c:	fcf42c23          	sw	a5,-40(s0)
   10320:	000157b7          	lui	a5,0x15
   10324:	9817871b          	addiw	a4,a5,-1663 # 14981 <__global_pointer$+0x2a19>
   10328:	fe442783          	lw	a5,-28(s0)
   1032c:	00058613          	mv	a2,a1
   10330:	18e637af          	sc.d	a5,a4,(a2)
   10334:	fef42223          	sw	a5,-28(s0)
   10338:	fe442783          	lw	a5,-28(s0)
   1033c:	00078513          	mv	a0,a5
   10340:	ea1ff0ef          	jal	ra,101e0 <print_d>
   10344:	00000793          	li	a5,0
   10348:	00078513          	mv	a0,a5
   1034c:	02813083          	ld	ra,40(sp)
   10350:	02013403          	ld	s0,32(sp)
   10354:	03010113          	addi	sp,sp,48
   10358:	00008067          	ret

000000000001035c <__libc_init_array>:
   1035c:	fe010113          	addi	sp,sp,-32
   10360:	00813823          	sd	s0,16(sp)
   10364:	000117b7          	lui	a5,0x11
   10368:	00011437          	lui	s0,0x11
   1036c:	01213023          	sd	s2,0(sp)
   10370:	75078793          	addi	a5,a5,1872 # 11750 <__init_array_start>
   10374:	75040713          	addi	a4,s0,1872 # 11750 <__init_array_start>
   10378:	00113c23          	sd	ra,24(sp)
   1037c:	00913423          	sd	s1,8(sp)
   10380:	40e78933          	sub	s2,a5,a4
   10384:	02e78263          	beq	a5,a4,103a8 <__libc_init_array+0x4c>
   10388:	40395913          	srai	s2,s2,0x3
   1038c:	75040413          	addi	s0,s0,1872
   10390:	00000493          	li	s1,0
   10394:	00043783          	ld	a5,0(s0)
   10398:	00148493          	addi	s1,s1,1
   1039c:	00840413          	addi	s0,s0,8
   103a0:	000780e7          	jalr	a5
   103a4:	ff24e8e3          	bltu	s1,s2,10394 <__libc_init_array+0x38>
   103a8:	00011437          	lui	s0,0x11
   103ac:	000117b7          	lui	a5,0x11
   103b0:	76078793          	addi	a5,a5,1888 # 11760 <__do_global_dtors_aux_fini_array_entry>
   103b4:	75040713          	addi	a4,s0,1872 # 11750 <__init_array_start>
   103b8:	40e78933          	sub	s2,a5,a4
   103bc:	40395913          	srai	s2,s2,0x3
   103c0:	02e78063          	beq	a5,a4,103e0 <__libc_init_array+0x84>
   103c4:	75040413          	addi	s0,s0,1872
   103c8:	00000493          	li	s1,0
   103cc:	00043783          	ld	a5,0(s0)
   103d0:	00148493          	addi	s1,s1,1
   103d4:	00840413          	addi	s0,s0,8
   103d8:	000780e7          	jalr	a5
   103dc:	ff24e8e3          	bltu	s1,s2,103cc <__libc_init_array+0x70>
   103e0:	01813083          	ld	ra,24(sp)
   103e4:	01013403          	ld	s0,16(sp)
   103e8:	00813483          	ld	s1,8(sp)
   103ec:	00013903          	ld	s2,0(sp)
   103f0:	02010113          	addi	sp,sp,32
   103f4:	00008067          	ret

00000000000103f8 <memset>:
   103f8:	00f00313          	li	t1,15
   103fc:	00050713          	mv	a4,a0
   10400:	02c37a63          	bgeu	t1,a2,10434 <memset+0x3c>
   10404:	00f77793          	andi	a5,a4,15
   10408:	0a079063          	bnez	a5,104a8 <memset+0xb0>
   1040c:	06059e63          	bnez	a1,10488 <memset+0x90>
   10410:	ff067693          	andi	a3,a2,-16
   10414:	00f67613          	andi	a2,a2,15
   10418:	00e686b3          	add	a3,a3,a4
   1041c:	00b73023          	sd	a1,0(a4)
   10420:	00b73423          	sd	a1,8(a4)
   10424:	01070713          	addi	a4,a4,16
   10428:	fed76ae3          	bltu	a4,a3,1041c <memset+0x24>
   1042c:	00061463          	bnez	a2,10434 <memset+0x3c>
   10430:	00008067          	ret
   10434:	40c306b3          	sub	a3,t1,a2
   10438:	00269693          	slli	a3,a3,0x2
   1043c:	00000297          	auipc	t0,0x0
   10440:	005686b3          	add	a3,a3,t0
   10444:	00c68067          	jr	12(a3)
   10448:	00b70723          	sb	a1,14(a4)
   1044c:	00b706a3          	sb	a1,13(a4)
   10450:	00b70623          	sb	a1,12(a4)
   10454:	00b705a3          	sb	a1,11(a4)
   10458:	00b70523          	sb	a1,10(a4)
   1045c:	00b704a3          	sb	a1,9(a4)
   10460:	00b70423          	sb	a1,8(a4)
   10464:	00b703a3          	sb	a1,7(a4)
   10468:	00b70323          	sb	a1,6(a4)
   1046c:	00b702a3          	sb	a1,5(a4)
   10470:	00b70223          	sb	a1,4(a4)
   10474:	00b701a3          	sb	a1,3(a4)
   10478:	00b70123          	sb	a1,2(a4)
   1047c:	00b700a3          	sb	a1,1(a4)
   10480:	00b70023          	sb	a1,0(a4)
   10484:	00008067          	ret
   10488:	0ff5f593          	zext.b	a1,a1
   1048c:	00859693          	slli	a3,a1,0x8
   10490:	00d5e5b3          	or	a1,a1,a3
   10494:	01059693          	slli	a3,a1,0x10
   10498:	00d5e5b3          	or	a1,a1,a3
   1049c:	02059693          	slli	a3,a1,0x20
   104a0:	00d5e5b3          	or	a1,a1,a3
   104a4:	f6dff06f          	j	10410 <memset+0x18>
   104a8:	00279693          	slli	a3,a5,0x2
   104ac:	00000297          	auipc	t0,0x0
   104b0:	005686b3          	add	a3,a3,t0
   104b4:	00008293          	mv	t0,ra
   104b8:	f98680e7          	jalr	-104(a3)
   104bc:	00028093          	mv	ra,t0
   104c0:	ff078793          	addi	a5,a5,-16
   104c4:	40f70733          	sub	a4,a4,a5
   104c8:	00f60633          	add	a2,a2,a5
   104cc:	f6c374e3          	bgeu	t1,a2,10434 <memset+0x3c>
   104d0:	f3dff06f          	j	1040c <memset+0x14>

00000000000104d4 <__call_exitprocs>:
   104d4:	fb010113          	addi	sp,sp,-80
   104d8:	03413023          	sd	s4,32(sp)
   104dc:	f481ba03          	ld	s4,-184(gp) # 11eb0 <_global_impure_ptr>
   104e0:	03213823          	sd	s2,48(sp)
   104e4:	04113423          	sd	ra,72(sp)
   104e8:	1f8a3903          	ld	s2,504(s4)
   104ec:	04813023          	sd	s0,64(sp)
   104f0:	02913c23          	sd	s1,56(sp)
   104f4:	03313423          	sd	s3,40(sp)
   104f8:	01513c23          	sd	s5,24(sp)
   104fc:	01613823          	sd	s6,16(sp)
   10500:	01713423          	sd	s7,8(sp)
   10504:	01813023          	sd	s8,0(sp)
   10508:	04090063          	beqz	s2,10548 <__call_exitprocs+0x74>
   1050c:	00050b13          	mv	s6,a0
   10510:	00058b93          	mv	s7,a1
   10514:	00100a93          	li	s5,1
   10518:	fff00993          	li	s3,-1
   1051c:	00892483          	lw	s1,8(s2)
   10520:	fff4841b          	addiw	s0,s1,-1
   10524:	02044263          	bltz	s0,10548 <__call_exitprocs+0x74>
   10528:	00349493          	slli	s1,s1,0x3
   1052c:	009904b3          	add	s1,s2,s1
   10530:	040b8463          	beqz	s7,10578 <__call_exitprocs+0xa4>
   10534:	2084b783          	ld	a5,520(s1)
   10538:	05778063          	beq	a5,s7,10578 <__call_exitprocs+0xa4>
   1053c:	fff4041b          	addiw	s0,s0,-1
   10540:	ff848493          	addi	s1,s1,-8
   10544:	ff3416e3          	bne	s0,s3,10530 <__call_exitprocs+0x5c>
   10548:	04813083          	ld	ra,72(sp)
   1054c:	04013403          	ld	s0,64(sp)
   10550:	03813483          	ld	s1,56(sp)
   10554:	03013903          	ld	s2,48(sp)
   10558:	02813983          	ld	s3,40(sp)
   1055c:	02013a03          	ld	s4,32(sp)
   10560:	01813a83          	ld	s5,24(sp)
   10564:	01013b03          	ld	s6,16(sp)
   10568:	00813b83          	ld	s7,8(sp)
   1056c:	00013c03          	ld	s8,0(sp)
   10570:	05010113          	addi	sp,sp,80
   10574:	00008067          	ret
   10578:	00892783          	lw	a5,8(s2)
   1057c:	0084b703          	ld	a4,8(s1)
   10580:	fff7879b          	addiw	a5,a5,-1
   10584:	06878263          	beq	a5,s0,105e8 <__call_exitprocs+0x114>
   10588:	0004b423          	sd	zero,8(s1)
   1058c:	fa0708e3          	beqz	a4,1053c <__call_exitprocs+0x68>
   10590:	31092783          	lw	a5,784(s2)
   10594:	008a96bb          	sllw	a3,s5,s0
   10598:	00892c03          	lw	s8,8(s2)
   1059c:	00d7f7b3          	and	a5,a5,a3
   105a0:	0007879b          	sext.w	a5,a5
   105a4:	02079263          	bnez	a5,105c8 <__call_exitprocs+0xf4>
   105a8:	000700e7          	jalr	a4
   105ac:	00892703          	lw	a4,8(s2)
   105b0:	1f8a3783          	ld	a5,504(s4)
   105b4:	01871463          	bne	a4,s8,105bc <__call_exitprocs+0xe8>
   105b8:	f92782e3          	beq	a5,s2,1053c <__call_exitprocs+0x68>
   105bc:	f80786e3          	beqz	a5,10548 <__call_exitprocs+0x74>
   105c0:	00078913          	mv	s2,a5
   105c4:	f59ff06f          	j	1051c <__call_exitprocs+0x48>
   105c8:	31492783          	lw	a5,788(s2)
   105cc:	1084b583          	ld	a1,264(s1)
   105d0:	00d7f7b3          	and	a5,a5,a3
   105d4:	0007879b          	sext.w	a5,a5
   105d8:	00079c63          	bnez	a5,105f0 <__call_exitprocs+0x11c>
   105dc:	000b0513          	mv	a0,s6
   105e0:	000700e7          	jalr	a4
   105e4:	fc9ff06f          	j	105ac <__call_exitprocs+0xd8>
   105e8:	00892423          	sw	s0,8(s2)
   105ec:	fa1ff06f          	j	1058c <__call_exitprocs+0xb8>
   105f0:	00058513          	mv	a0,a1
   105f4:	000700e7          	jalr	a4
   105f8:	fb5ff06f          	j	105ac <__call_exitprocs+0xd8>

00000000000105fc <atexit>:
   105fc:	00050593          	mv	a1,a0
   10600:	00000693          	li	a3,0
   10604:	00000613          	li	a2,0
   10608:	00000513          	li	a0,0
   1060c:	0600006f          	j	1066c <__register_exitproc>

0000000000010610 <__libc_fini_array>:
   10610:	fe010113          	addi	sp,sp,-32
   10614:	00813823          	sd	s0,16(sp)
   10618:	000117b7          	lui	a5,0x11
   1061c:	00011437          	lui	s0,0x11
   10620:	76078793          	addi	a5,a5,1888 # 11760 <__do_global_dtors_aux_fini_array_entry>
   10624:	76840413          	addi	s0,s0,1896 # 11768 <impure_data>
   10628:	40f40433          	sub	s0,s0,a5
   1062c:	00913423          	sd	s1,8(sp)
   10630:	00113c23          	sd	ra,24(sp)
   10634:	40345493          	srai	s1,s0,0x3
   10638:	02048063          	beqz	s1,10658 <__libc_fini_array+0x48>
   1063c:	ff840413          	addi	s0,s0,-8
   10640:	00f40433          	add	s0,s0,a5
   10644:	00043783          	ld	a5,0(s0)
   10648:	fff48493          	addi	s1,s1,-1
   1064c:	ff840413          	addi	s0,s0,-8
   10650:	000780e7          	jalr	a5
   10654:	fe0498e3          	bnez	s1,10644 <__libc_fini_array+0x34>
   10658:	01813083          	ld	ra,24(sp)
   1065c:	01013403          	ld	s0,16(sp)
   10660:	00813483          	ld	s1,8(sp)
   10664:	02010113          	addi	sp,sp,32
   10668:	00008067          	ret

000000000001066c <__register_exitproc>:
   1066c:	f481b703          	ld	a4,-184(gp) # 11eb0 <_global_impure_ptr>
   10670:	1f873783          	ld	a5,504(a4)
   10674:	06078063          	beqz	a5,106d4 <__register_exitproc+0x68>
   10678:	0087a703          	lw	a4,8(a5)
   1067c:	01f00813          	li	a6,31
   10680:	08e84663          	blt	a6,a4,1070c <__register_exitproc+0xa0>
   10684:	02050863          	beqz	a0,106b4 <__register_exitproc+0x48>
   10688:	00371813          	slli	a6,a4,0x3
   1068c:	01078833          	add	a6,a5,a6
   10690:	10c83823          	sd	a2,272(a6)
   10694:	3107a883          	lw	a7,784(a5)
   10698:	00100613          	li	a2,1
   1069c:	00e6163b          	sllw	a2,a2,a4
   106a0:	00c8e8b3          	or	a7,a7,a2
   106a4:	3117a823          	sw	a7,784(a5)
   106a8:	20d83823          	sd	a3,528(a6)
   106ac:	00200693          	li	a3,2
   106b0:	02d50863          	beq	a0,a3,106e0 <__register_exitproc+0x74>
   106b4:	00270693          	addi	a3,a4,2
   106b8:	00369693          	slli	a3,a3,0x3
   106bc:	0017071b          	addiw	a4,a4,1
   106c0:	00e7a423          	sw	a4,8(a5)
   106c4:	00d787b3          	add	a5,a5,a3
   106c8:	00b7b023          	sd	a1,0(a5)
   106cc:	00000513          	li	a0,0
   106d0:	00008067          	ret
   106d4:	20070793          	addi	a5,a4,512
   106d8:	1ef73c23          	sd	a5,504(a4)
   106dc:	f9dff06f          	j	10678 <__register_exitproc+0xc>
   106e0:	3147a683          	lw	a3,788(a5)
   106e4:	00000513          	li	a0,0
   106e8:	00c6e6b3          	or	a3,a3,a2
   106ec:	30d7aa23          	sw	a3,788(a5)
   106f0:	00270693          	addi	a3,a4,2
   106f4:	00369693          	slli	a3,a3,0x3
   106f8:	0017071b          	addiw	a4,a4,1
   106fc:	00e7a423          	sw	a4,8(a5)
   10700:	00d787b3          	add	a5,a5,a3
   10704:	00b7b023          	sd	a1,0(a5)
   10708:	00008067          	ret
   1070c:	fff00513          	li	a0,-1
   10710:	00008067          	ret

0000000000010714 <_exit>:
   10714:	05d00893          	li	a7,93
   10718:	00000073          	ecall
   1071c:	00054463          	bltz	a0,10724 <_exit+0x10>
   10720:	0000006f          	j	10720 <_exit+0xc>
   10724:	ff010113          	addi	sp,sp,-16
   10728:	00813023          	sd	s0,0(sp)
   1072c:	00050413          	mv	s0,a0
   10730:	00113423          	sd	ra,8(sp)
   10734:	4080043b          	negw	s0,s0
   10738:	00c000ef          	jal	ra,10744 <__errno>
   1073c:	00852023          	sw	s0,0(a0)
   10740:	0000006f          	j	10740 <_exit+0x2c>

0000000000010744 <__errno>:
   10744:	f581b503          	ld	a0,-168(gp) # 11ec0 <_impure_ptr>
   10748:	00008067          	ret

Disassembly of section .eh_frame:

000000000001174c <__FRAME_END__>:
   1174c:	0000                	.2byte	0x0
	...

Disassembly of section .init_array:

0000000000011750 <__init_array_start>:
   11750:	0118                	.2byte	0x118
   11752:	0001                	.2byte	0x1
   11754:	0000                	.2byte	0x0
	...

0000000000011758 <__frame_dummy_init_array_entry>:
   11758:	01c0                	.2byte	0x1c0
   1175a:	0001                	.2byte	0x1
   1175c:	0000                	.2byte	0x0
	...

Disassembly of section .fini_array:

0000000000011760 <__do_global_dtors_aux_fini_array_entry>:
   11760:	017c                	.2byte	0x17c
   11762:	0001                	.2byte	0x1
   11764:	0000                	.2byte	0x0
	...

Disassembly of section .data:

0000000000011768 <impure_data>:
	...
   11770:	1ca0                	.2byte	0x1ca0
   11772:	0001                	.2byte	0x1
   11774:	0000                	.2byte	0x0
   11776:	0000                	.2byte	0x0
   11778:	1d50                	.2byte	0x1d50
   1177a:	0001                	.2byte	0x1
   1177c:	0000                	.2byte	0x0
   1177e:	0000                	.2byte	0x0
   11780:	1e00                	.2byte	0x1e00
   11782:	0001                	.2byte	0x1
	...
   11850:	0001                	.2byte	0x1
   11852:	0000                	.2byte	0x0
   11854:	0000                	.2byte	0x0
   11856:	0000                	.2byte	0x0
   11858:	330e                	.2byte	0x330e
   1185a:	abcd                	.2byte	0xabcd
   1185c:	1234                	.2byte	0x1234
   1185e:	e66d                	.2byte	0xe66d
   11860:	deec                	.2byte	0xdeec
   11862:	0005                	.2byte	0x5
   11864:	0000000b          	.4byte	0xb
	...

Disassembly of section .sdata:

0000000000011eb0 <_global_impure_ptr>:
   11eb0:	1768                	.2byte	0x1768
   11eb2:	0001                	.2byte	0x1
   11eb4:	0000                	.2byte	0x0
	...

0000000000011eb8 <__dso_handle>:
	...

0000000000011ec0 <_impure_ptr>:
   11ec0:	1768                	.2byte	0x1768
   11ec2:	0001                	.2byte	0x1
   11ec4:	0000                	.2byte	0x0
	...

Disassembly of section .bss:

0000000000011ec8 <completed.1>:
	...

0000000000011ed0 <object.0>:
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
