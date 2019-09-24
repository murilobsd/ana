# Ana
> Gerador estático de websites. Simples, rápido, seguro e o melhor de tudo livre.

*ainda em desenvolvimento*

## Porque Ana?

A verdade é que estava de saco cheio dos geradores "famosos"
tornando o que era para ser algo simples em algo complicado. Alguns geradores dizem ser
"super rápidos", outros dizem suportar X, Y, Z, mas que merda é essa; um gerador
recebe algo e gera uma saída no nosso caso *HTML* nem mais nem menos. 

*Caso você goste de frescuras Ana não é para você.*

É fato hoje existem muitos geradores estáticos mas na sua maioria se não todos
pecam em ser simples, Ana foi baseado no ssg de Roman feito em Shell Script,
por favor olhem.

O binário possui 400Kb *(com markdown)*, você pode publicar seu website em
praticamente qualquer lugar, darei algumas dicas abaixo ;-)

### Resumão

Pró:
- menor que a framework css Bootstrap4
- suporta pledge
- suporta markdown
- possui header e footer direto no código
- "rápido" 5000 arquivos em 3s

Contra:
- ainda não faz cache
- ainda não gera rss
- funcionar no OpenBSD, não testei no Free, Net, Linux, Win e MacOS
- ainda não possui título nas páginas
- ainda não é um blogger

## Índice

- [Instalação](#instalação)
  - [Requisitos](#requisitos)
  - [Instalação Binário](#instalação-binário)
  - [Instalação via Repo](#instalação-via-repo)
- [Uso](#uso)
- [Exemplo](#exemplo-de-uso)
- [Publicar](#publicar)
- [LICENSA](LICENSE)

## Instalação

Eu recomendo fortemente que você tente compilar o código do repositório, mas
caso tenha preguiça, ou é ansioso como eu pode baixar o binário em
https://ana.m0x.ru/d/ana-amd64-obsd ao menos cheque o checksum. 

### Requisitos

Caso você queira usar markdown necessita instalar lowndown. Ana foi usado
somente no sistema operacional OpenBSD acho que você terá dificuldades em usar
em outros sistema, mas caso você tenha interesse pode portar para colocarmos
aqui.

Para instalar o lowdown no OpenBSD

```
doas pkg_add -v lowndown
```

### Instalação binário

A instalação apresentada abaixo foi feito no OpenBSD.

```
mkdir -p ~/bin
ftp -Vo ~/bin/ana https://ana.m0x.ru/d/ana-amd64-obsd
chmod +x ~/bin/ana
export PATH=$HOME/bin:$PATH
ana
usage: ana src dst
```

### Instalação via repo

```
git clone https://github.com/murilobsd/ana.git
cd ana
doas make install
ana
usage: ana src dst
```

## Uso

Basicamente passa o caminho onde estão os seus arquivos html's, markdown's,
arquivos estáticos (css, img...) e o destino.

**Arquivos/Pastas considerados ocultos (".nomearquivo") não serão copiados.**

Você pode adicionar um arquivo que represente o header do seu website e também
um que represente o footer esses arquivos tem que estar dentro da pasta de
origem com os respectivos nomes, por exemplo caso sua pasta de origem tivesse o
nome **src**

- src/_header.html
- src/_footer.html

Além de ignorar arquivos ocultos você pode adicionar pastas ou caminho completo
do nome do arquivo a ser ignorado no arquivo **.anaignore**.
```
ana
usage: ana src dst
```

## Exemplo de uso
Os arquivos **_header.html** e **_footer.html** não são obrigatórios, mas no exemplo abaixo irei
demosntrar como seria gerar o website com todas as opções.

```
mkdir -p {src,dst}

echo "<html><head><title>Meu Site</title></head><body>" > src/_header.html
echo "</body></html>" > src/_footer.html
echo "# Oi" > src/index.md

echo "<p>ignorado</p>" src/ignorado.html
echo "src/ignorado.html" >> src/.anaignore

ana src/ dst/
[ana] src/ --> dst/
[ana] read header file
[ana] read footer file
[ana] read ignore file
Processing 1 on 0.01 seconds

cat dst/index.html
<html><head><title>Meu Site</tile></head><body>
<h1 id="oi">Oi</h1>
</body></html>

```

## Publicar

Por ser conteúdo estático você pode praticamente hospedar em qualquer lugar de
forma muito simples e rápida.

### S3

### Firebase

### GitHub
