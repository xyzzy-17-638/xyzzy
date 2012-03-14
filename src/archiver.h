#ifndef _ARCHIVER_H_
# define _ARCHIVER_H_

# include "arc-if.h"

class ArchiverP
{
  const char *const *const ar_csuffixes;
  const char *const *const ar_esuffixes;
  const char *const *const ar_rsuffixes;

public:
  ArchiverP (const char *const *csuffixes,
             const char *const *esuffixes,
             const char *const *rsuffixes)
       : ar_csuffixes (csuffixes),
         ar_esuffixes (esuffixes), ar_rsuffixes (rsuffixes) {}
  virtual int match_suffix (const char *, int, const char *const *) const = 0;
  virtual int check_archive (const char *) const = 0;
  virtual int reliable_checker_p () const =0;
  virtual int extract (HWND, const char *, const char *, const char *) const = 0;
  virtual int create (HWND, const char *, const char *) const = 0;
  virtual int create_sfx (HWND, const char *, const char *) const = 0;
  virtual int remove (HWND, const char *, const char *) const = 0;
  virtual const char *match_any () const = 0;
  virtual void puts_create (FILE *, char *, const char *) const = 0;
  virtual void puts_extract (FILE *, char *) const = 0;
  virtual   WORD get_version () const = 0;
  virtual   WORD get_sub_version () const = 0;
  virtual BOOL config_dialog (HWND hwnd, LPSTR buf, int mode) const = 0;
  virtual lisp list (const char *path, int file_name_only) const = 0;


  int match_csuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_csuffixes);}
  int match_esuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_esuffixes);}
  int match_rsuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_rsuffixes);}
};

class ArchiverPImpl : public ArchiverP
{
public:
  const ArchiverInterface &ar_interface;
protected:
  static const char *const null_suffixes[];
  ArchiverPImpl (const ArchiverInterface &i,
             const char *const *csuffixes,
             const char *const *esuffixes,
             const char *const *rsuffixes)
       : ar_interface (i), ArchiverP(csuffixes, esuffixes, rsuffixes) {}
  int doit (HWND, const char *) const;
  int extract (HWND, const char *) const;
  int extract_noresp (HWND, const char *, int, const char *) const;
  int extract (HWND, const char *, const char *, const char *,
               const char *, const char *, const char *) const;
  int extract1 (HWND, const char *, const char *, const char *,
                const char *, const char *, const char *, char *) const;
  int create (HWND, const char *, const char *, const char *, const char *) const;
  int create1 (HWND, const char *, const char *, const char *, const char *, char *) const;
  int remove (HWND, const char *, const char *, const char *, const char *) const;
  int create_sfx (HWND, const char *, const char *, const char *) const;
  static void sepmap (char *, int, int);
  static void sepsl (char *path) {sepmap (path, '\\', '/');}
  static void sepbacksl (char *path) {sepmap (path, '/', '\\');}
public:
  virtual int check_archive (const char *) const;
  virtual int reliable_checker_p () const {return 1;}
  virtual int extract (HWND, const char *, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int create (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int create_sfx (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int remove (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int match_suffix (const char *, int, const char *const *) const;
  virtual void puts_create (FILE *, char *, const char *) const;
  virtual void puts_extract (FILE *, char *) const;
  virtual const char *match_any () const {return "";}
  virtual int post_open (HARC harc) const {return 0;}

  virtual   WORD get_version () const;
  virtual   WORD get_sub_version () const;
  virtual BOOL config_dialog (HWND hwnd, LPSTR buf, int mode) const;
  virtual lisp list (const char *path, int file_name_only) const;
};

class Ish: public ArchiverPImpl
{
  IshInterface ish;
  static const char *const suffixes[];
public:
  Ish () : ArchiverPImpl (ish, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int reliable_checker_p () const {return 0;}
};

class Tar: public ArchiverPImpl
{
  TarInterface tar;
  static const char *const suffixes[];
public:
  Tar () : ArchiverPImpl (tar, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual void map_sl (char *path) const {sepsl (path);}
};

class Arj: public ArchiverPImpl
{
  UnarjInterface arj;
  static const char *const suffixes[];
public:
  Arj () : ArchiverPImpl (arj, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class Lha: public ArchiverPImpl
{
  UnlhaInterface lha;
  static const char *const csuffixes[];
  static const char *const esuffixes[];
public:
  Lha () : ArchiverPImpl (lha, csuffixes, esuffixes, csuffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int check_archive (const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual void puts_extract (FILE *, char *) const;
  virtual int create_sfx (HWND, const char *, const char *) const;
  virtual const char *match_any () const {return "*.*";}
};

class Unzip: public ArchiverPImpl
{
  UnzipInterface unzip;
  static const char *const esuffixes[];
public:
  Unzip () : ArchiverPImpl (unzip, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual void map_sl (char *path) const {sepsl (path);}
  virtual void puts_extract (FILE *, char *) const;
};


class Zip: public ArchiverPImpl
{
  ZipInterface zip;
  const Unzip &unzip;
  static const char *const csuffixes[];
public:
  Zip (const Unzip &unzip_)
       : ArchiverPImpl (zip, csuffixes, null_suffixes, csuffixes), unzip (unzip_) {}
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual int check_archive (const char *) const;
  virtual void puts_create (FILE *, char *, const char *) const;
};

class Cab: public ArchiverPImpl
{
  CabInterface cab;
  static const char *const csuffixes[];
  static const char *const esuffixes[];
public:
  Cab () : ArchiverPImpl (cab, csuffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
};

class Unrar: public ArchiverPImpl
{
  UnrarInterface unrar;
  static const char *const esuffixes[];
public:
  Unrar () : ArchiverPImpl (unrar, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class Bga: public ArchiverPImpl
{
  BgaInterface bga;
  static const char *const suffixes[];
public:
  Bga () : ArchiverPImpl (bga, suffixes, suffixes, suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual const char *match_any () const {return "*.*";}
};

class Yz1: public ArchiverPImpl
{
  Yz1Interface yz1;
  static const char *const suffixes[];
public:
  Yz1 () : ArchiverPImpl (yz1, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int post_open (HARC harc) const
    {return yz1.set_default_passwd (harc, 0);}
};

class UnGCA: public ArchiverPImpl
{
  UnGCAInterface ungca;
  static const char *const esuffixes[];
public:
  UnGCA () : ArchiverPImpl (ungca, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class SevenZip: public ArchiverPImpl
{
  SevenZipInterface seven_zip;
  static const char *const suffixes[];
public:
  SevenZip () : ArchiverPImpl (seven_zip, suffixes, suffixes, suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual void puts_create (FILE *, char *, const char *) const;
};

class Archiver
{
protected:
  Ish a_ish;
  Tar a_tar;
  Arj a_arj;
  Lha a_lha;
  Unzip a_unzip;
  Zip a_zip;
  Cab a_cab;
  Unrar a_unrar;
  Bga a_bga;
  Yz1 a_yz1;
  UnGCA a_ungca;
  SevenZip a_seven_zip;
  enum {NARCS = 12};
  ArchiverP *arcs[NARCS];

  static int check_file_size (const char *);
public:
  Archiver ();
  const ArchiverP *get_creator (const char *) const;
  const ArchiverP *get_extractor (const char *) const;
  const ArchiverP *get_remover (const char *) const;
  const ArchiverP *check_archive (const char *) const;
  int extract (HWND, const char *, const char *, const char *) const;
  int create (HWND, const char *, const char *) const;
  int create_sfx (HWND, const char *, const char *) const;
  lisp list (const char *, int) const;
  static int get_version (const ArchiverP &, char *);
  static int config_dialog (const ArchiverP &, HWND hwnd, int mode);
  const ArchiverP *get (lisp) const;
};

#endif
