.section .text
.global switch_to

# void switch_to(process_t *prev, process_t *next);
# 4(%esp) = prev, 8(%esp) = next
switch_to:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl 20(%esp), %eax   # prev (偏移调整: 4*4+4=20)
    movl 24(%esp), %edx   # next
    movl %esp, 8(%eax)   # 保存prev->esp (结构体中esp的偏移为12)
    movl 8(%edx), %esp   # 加载next->esp
    movl %edx, %eax       # 返回next指针

    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
