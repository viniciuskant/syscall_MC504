#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define MC504FS_MAGIC 0x20df84ab // aleatório


// https://www.kernel.org/doc/html/latest/filesystems/vfs.html
// https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part1.html

// VFS (Virtual Filesystem) é uma abstração do kernel para lidar com as syscalls relacionadas a sistemas de arquivos, ou seja, é algo genérico para facilitara a implementação do sistema de arquivos.
//
// Há três tipos de sistemas de arquivos:
// * de Disco
// * de Rede (nfs, smbfs, ncp...)
// * Virtuais (procfs, sysfs...)
//

/*
    Conceitos básicos:

    * SuperBlock:
        O SuperBlock é uma estrutura de dados que contém informações sobre o sistema de arquivos como um todo.
        Ele armazena dados como o tamanho do sistema de arquivos, o número e localização dos  inodes e  blocos,
        informações sobre o gerenciamento de espaço livre, e outros metadados globais. É essencial para
        inicializar e gerenciar o sistema de arquivos. Pelo o que entendi seria uma espécie de cabeçalho.

        Em sistema de arquvios de disco o superblock corresponde ao primeiro bloco do disco e em VFS os superblocks do sistema estão em uma lista de struct do tipo super_block e com os métodso em outra struct chamada super_operations.

    * Inode (index node):
        O Inode é uma estrutura que representa um arquivo ou diretório no sistema de arquivos. Ele contém
        informações como permissões de acesso, proprietário, tamanho, timestamps (criação, modificação, acesso e ponteiros para os blocos de dados que armazenam o conteúdo do arquivo. Cada arquivo ou diretório possui um inode único.

        Uma espécie de "superblock" dos diretórios e arquivos, se não me engano é por isso que mover uma arquvio ou diretório dentro de um mesmo disco é fácil, pois apenas basta alterar as informações do inode e isso

        Curisodade, os inodes de 1 a 10 são reservados para propósitos especiais, não existe inode 0 (zero) e o inode 2 é a raiz (o barra). Para saber o inode de algo, basta executar """ls -i"

    * Localização:
        

    * File:
        A estrutura File representa uma instância ABERTA de um arquivo. Ela é usada para gerenciar o estado
        de um arquivo enquanto ele está aberto, incluindo o ponteiro de posição atual, flags de acesso,
        e informações específicas da sessão de uso. É uma abstração usada pelo kernel para interagir com
        arquivos abertos. E como representa umas instancia aberta de um arquivo, ela não possui uma correspondência no disco físico. Pelo o que entendi, seria uma espécie de inode para memória (ram).

    * Dentry (directory entry):
        A Dentry é usada para gerenciar a hierarquia de diretórios e nomes de arquivos. (funciona como uma árvore?)
        Ela representa uma entrada em um diretório e é usada para mapear nomes de arquivos para inodes.
        Além disso, ela ajuda a otimizar o acesso ao sistema de arquivos através de caching, reduzindo
        a necessidade de buscas repetidas.

        Ela possui algunas campos, entre eles:
            Um inteiro que representa o Inode
            Uma string que representa o nome

        O path "/bin/vi" possui 3 dentry, "/", "bin" e "vi" por exemplo.

    *  Redistro e "desregistro" de filesystem:
        Casa fs é um módulo do kernel que pode ser carregado ao kernel ou não, atualemtne há mais de 50 fs diferetes, mas geralmente cada implementação/sistema não possui mais de 5 ou 6 desse módulos carregados. Para poder fazer esse carregamento é necessário descrever o sistema de arquivos na struct file_system_type.

        Isso na pratica significa na função init dos fs fazer a chamada da função register_filesystem, passando como argumetno o nome da struct file_system_type do seu fs.

    * Funções mount e kill_sb:
        ** Mount:
            Geralmete essa função é uma casca que chama outra funções:
                mount_bdev: monta o sistema de arquvios em um dispositivo; 
                mount_single: não entendi muito bem, será que está ralacioanda ao ".owner", para garantir que só seja montado uma única vez?   
                mount_nodev: para montar sistemas de arquivos que não estão em um disco físico
                mount_pseudo: uma função de auxiliar para pseudo fs (filesystem que não podem ser montados)
            para fazer a inicialização e retornar qual o dentry desse nova instancia do fs.


            Há uma chamda da função fill_super() (não entendi muito bem)


        ** kill_sb:
            Para desmontar um sistema de arquvios o kernel chama a função kill_sb, que também é uma casca para as seguintes funções:
                * kill_block_super: desmonta o sistema de arquivos no dispositivo           
                * kill_anon_super: desmonta um sistema de arquvios virtual
                * kill_litter_super: desmonta um sistema de arquvios que não está em um disco, mas sim na memória;

    * SuperBlock no VFS:
        O SuperBlock é uma entidade física (no disco) e uma entidade no VFS (estrutura `struct super_block`). Ele contém apenas metainformações e é usado para ler e escrever metadados no disco, como inodes e entradas de diretórios. 
        
        Ele inclui informações sobre o dispositivo de bloco utilizado, a lista de inodes, um ponteiro para o inode do diretório raiz do sistema de arquivos e um ponteiro para o próprio SuperBlock.

        ** A struct super_block define as seguintes coias:

            struct super_block {
                dev_t                   s_dev;              // identifier 
                unsigned char           s_blocksize_bits;   // block size in bits 
                unsigned long           s_blocksize;        // block size in bytes 
                unsigned char           s_dirt;             // dirty flag 
                loff_t                  s_maxbytes;         // max file size 
                struct file_system_type *s_type;            // filesystem type 
                struct super_operations *s_op;              // superblock methods 
                //...
                unsigned long           s_flags;            // mount flags 
                unsigned long           s_magic;            // filesystem’s magic number 
                struct dentry           *s_root;            // directory mount point 
                //...
                char                    s_id[32];           // informational name 
                void                    *s_fs_info;         // filesystem private info 
            };
        
        ** A struct super_block:
            struct super_operations {
                //...
                int (*write_inode) (struct inode *, struct writeback_control *wbc);
                struct inode *(*alloc_inode)(struct super_block *sb);
                void (*destroy_inode)(struct inode *);

                void (*put_super) (struct super_block *);
                int (*statfs) (struct dentry *, struct kstatfs *);
                int (*remount_fs) (struct super_block *, int *, char *);
                //...
            };

        ** A função fill_super:
           Ela que preenche as informações da struct super_block, para isso podemos olha como exemplo a implementação da ramfs_fill_super()




    NÃO LI A PARTE E "Buffer cache" e "Functions and useful macros"

*/


/*
    Parte pratica:
        Para saber se esse novo fs está disponível:
            cat /proc/filesystems

*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC504");
MODULE_DESCRIPTION("MC504FS: Basic filesystem");
MODULE_VERSION("0.1");



static void mc504fs_kill_superblock(struct super_block *s);
static struct dentry *mc504fs_mount(struct file_system_type *fs_type,
            int flags, const char *dev_name, void *data);


// colocar uma struct file_system_type 
static struct file_system_type mc504_fs_type = {
    .owner = THIS_MODULE,       // evita que o módulo seja carregado duas vezes
    .name = "mc504fs",          // argumento passado para mount -t
    .mount = mc504fs_mount,     // função para ler o superbloco ao montar o sistema
    .kill_sb = mc504fs_kill_superblock,   // função para desmontar o sistema de arquivos
};


static struct inode *mc504fs_get_inode(struct super_block *sb, 
    const struct inode *dir, umode_t mode, dev_t dev) {
    struct inode *inode = new_inode(sb); // buscar kernel rcu na internet
    
    if (inode) {
    inode->i_ino = get_next_ino();
    inode_init_owner(&nop_mnt_idmap, inode, dir, mode);
    simple_inode_init_ts(inode);

    // usar o nfsctl.c como referencia
    switch (mode & S_IFMT) {
        case S_IFREG: 
            // por isso não consigo criar arquivos/escrever, preciso preencher isso
            break;
        case S_IFDIR:
            // se não especificar essa tabela de funções o filesystem não é montado
            // para esse exemplo tudo considerará apenas memória
            pr_info("Directory inode initialized\n");
            inode->i_op = &simple_dir_inode_operations;
            inode->i_fop = &simple_dir_operations;
            inc_nlink(inode);
            break;
        default:
            pr_err("mc504fs, deu merda\n");
            return NULL;
    }
    }

    return inode;
}


static int mc504fs_fill_super(struct super_block *sb, void *data, int silent){
    // de fosse de vdd, seria onde a gente leria o primeiro bloco desse disco

    // se fosse de vdd não criaríamos esse inode, mas sim leriamos isso do disco
    struct inode *inode;

    sb->s_magic = MC504FS_MAGIC;
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;

    // S_IFDIR: so por conta do case que está na função anterior
    inode = mc504fs_get_inode(sb, NULL, S_IFDIR, 0);
    sb->s_root = d_make_root(inode);
    if (!sb->s_root) // senão definir as tabelas de funções, isso pode dar errado, por isso esse if
        return -ENOMEM;
    
    return 0;
}

// função que será chamada pelo syscall... 
// user ... mount ... syscall... (algo) ... essa função
static struct dentry *mc504fs_mount(struct file_system_type *fs_type,
    int flags, const char *dev_name, void *data) {
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, mc504fs_fill_super);

    if (IS_ERR(ret))
        pr_err("Error mounting mc504fs\n");
    else
        pr_info("mc504fs is successfully mounted on [%s]\n", dev_name);

    return ret;
}

static void mc504fs_kill_superblock(struct super_block *s) {
    kill_block_super(s);
    pr_info("mc504fs superblock is killed\n");
}


static int __init mc504fs_init(void){
    int ret;
    ret = register_filesystem(&mc504_fs_type);
    if (ret)
        pr_err("Failed to register mc504fs. Error:[%d]\n", ret);
    else
        pr_info("mc504fs was successfully loaded");
    return 0;
}

static void __exit mc504fs_exit(void){   
    int ret;
    ret = unregister_filesystem(&mc504_fs_type);
    if (ret)
        pr_err("Failed to unregister mc504fs. Error:[%d]\n", ret);
    else
        pr_info("mc504fs was successfully unloaded\n");
}

module_init(mc504fs_init);
module_exit(mc504fs_exit);
